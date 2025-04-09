// Compile: nvcc vmz.cu -o vmz 
// Run:     ./vmz < net.mcc
// daze@acm.org
// max_cooperative_grid_blocks()

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/time.h>
#include <cuda.h>
#include <cuda_runtime.h>
#include <cooperative_groups.h>
#include <cuda_runtime_api.h>

// data length definitions

//#define _LONG_MU_

#ifdef _LONG_MU_

#define MUTY long
#define MUMAX LONG_MAX

#else

#define MUTY int
#define MUMAX INT_MAX

#endif

// end of data length definitions

using namespace cooperative_groups; 

#define MATRIX_SIZE(d1,d2,t) ((d1)*(d2)*(sizeof(t)))
#define VECTOR_SIZE(d1,t)    ((d1)*(sizeof(t)))

#define MOFF(i,j,d1,d2) ((d2)*(i)+(j))
#define MELT(x,i,j,d1,d2) (*((x)+MOFF(i,j,d1,d2)))

#define zmax(x,y) (((x)>(y))?(x):(y))
#define zmin(x,y) (((x)<(y))?(x):(y))

#define CHECK(call)                                                            \
{                                                                              \
    const cudaError_t error = call;                                            \
    if (error != cudaSuccess)                                                  \
    {                                                                          \
        fprintf(stderr, "Error: %s:%d, ", __FILE__, __LINE__);                 \
        fprintf(stderr, "code: %d, reason: %s\n", error,                       \
                cudaGetErrorString(error));                                    \
        exit(1);                                                               \
    }                                                                          \
}

inline double seconds()
{
    struct timeval tp;
    struct timezone tzp;
    int i = gettimeofday(&tp, &tzp);
    return ((double)tp.tv_sec + (double)tp.tv_usec * 1.e-6);
}

void read_matr_int(int *x,int m,int n)
{
  int i,j;
  for(i=0;i<m;i++)
  {
    for(j=0;j<n;j++)
      scanf("%d",&MELT(x,i,j,m,n));
  }
}

void read_vect_int(int *x,int m)
{
  int i;
  for(i=0;i<m;i++)
  {
    scanf("%d",x+i);
  }
}

void read_vect_long(long *x,int m)
{
  int i;
  for(i=0;i<m;i++)
  {
    scanf("%ld",x+i);
  }
}

void print_matr_int(int *x,int m,int n)
{
  int i,j;
  for(i=0;i<m;i++)
  {
    for(j=0;j<n;j++)
      printf("%10d ",MELT(x,i,j,m,n));
    printf("\n");
  }
}

void print_matr_long(long *x,int m,int n)
{
  int i,j;
  for(i=0;i<m;i++)
  {
    for(j=0;j<n;j++)
      printf("%10ld ",MELT(x,i,j,m,n));
    printf("\n");
  }
}

void print_vect_int(int *x,int m)
{
  int i;
  for(i=0;i<m;i++)
  {
    printf("%d ",x[i]);
  }
  printf("\n");
}

void print_vect_long(long *x,int m)
{
  int i;
  for(i=0;i<m;i++)
  {
    printf("%ld ",x[i]);
  }
  printf("\n");
}


struct type_f {
  MUTY c;
  int t;
  long k;
};

__global__ void run_sn(int m, int n, int mm, int *bi, int *bv, int *di, int *dv, MUTY *mu, MUTY *y, struct type_f *f, long maxk, int dbg)
{
  int pi = threadIdx.x;
  int t = blockIdx.x;
  int step;
  cooperative_groups::grid_group grid = cooperative_groups::this_grid();

  if( pi==0 && t==0 ) (f->k)=0;

  while( (f->k) < maxk )
  {
    // fire_arc
    MELT(y,pi,t,mm,n) = (MELT(bv,pi,t,mm,n)>0)? mu[MELT(bi,pi,t,mm,n)] / MELT(bv,pi,t,mm,n) : 
                        (MELT(bv,pi,t,mm,n)<0)? ((mu[MELT(bi,pi,t,mm,n)]>0)? 0: MUMAX): MUMAX;
    grid.sync();
 
    /* fire_trs
    if(pi==0)
    {
      
      MELT(y,mm,t,mm,n)=MELT(y,0,t,mm,n);
      for(int ppi=1;ppi<mm;ppi++)
        MELT(y,mm,t,mm,n)=zmin(MELT(y,mm,t,mm,n),MELT(y,ppi,t,mm,n));
    }
    grid.sync();*/
    
    // fire_trs, red
    for(step=1; step<mm; step*=2) {
      if(pi%(step*2)==0) {
	if(pi+step<mm) MELT(y,pi,t,mm,n)=zmin(MELT(y,pi,t,mm,n),MELT(y,pi+step,t,mm,n));
      }
      __syncthreads();
    }
//    grid.sync();

    /* choose_f_trs
    if( pi==0 && t==0 )
    {
      int tt;

      (f->c)=0; (f->t)=0;
	 
      for(tt=0; tt<n; tt++)
      {
        if(MELT(y,0,tt,mm,n)>0)
        {
          (f->c)=MELT(y,0,tt,mm,n); // firing multiplicity
          (f->t)=tt;   		     // firing transition number
          break;
        }
      }
    }
    grid.sync();
*/

    // choose_f_trs, red
    
    
      if(pi==0) MELT(y,1,t,mm,n)=t;
      if((t==0) && (pi==0)) (f->c)=0;
      grid.sync();
      for(step=1; ((step<n) && (f->c)==0); step*=2) {
        if(pi==0) {if(t%(step*2)==0) {
          if(MELT(y,0,t,mm,n)>0) {
    	    if(t==0) { (f->c)=MELT(y,0,t,mm,n); (f->t)=MELT(y,1,t,mm,n); }
          } else if(t+step<n) {
            if(MELT(y,0,t+step,mm,n)>0) {
	      MELT(y,0,t,mm,n)=MELT(y,0,t+step,mm,n);
	      MELT(y,1,t,mm,n)=MELT(y,1,t+step,mm,n);
	      if(t==0) { (f->c)=MELT(y,0,t,mm,n); (f->t)=MELT(y,1,t,mm,n); }
            } 
          }
          }
        }
        grid.sync();
      }
//      if(t==0 && pi==0) { (f->c)=MELT(y,0,t,mm,n); (f->t)=MELT(y,1,t,mm,n);}
//      grid.sync();
    // end of choose_f_trs, red
    
    // is somebody firing ?
    if((f->c)>0){ 
      if( t==0 ) // next_mu
      {
        if(MELT(bv,pi,f->t,mm,n)>0) mu[MELT(bi,pi,f->t,mm,n)]-=(f->c)*MELT(bv,pi,f->t,mm,n);
        if(MELT(dv,pi,f->t,mm,n)>0) mu[MELT(di,pi,f->t,mm,n)]+=(f->c)*MELT(dv,pi,f->t,mm,n);
      }
    } else break;
    if( pi==0 && t==0 ) (f->k)++;
    grid.sync();

  } // end of while

} // end of run_sn



int main(int argc, char * argv[])
{
  int m, n, mm;
  int *bi, *bv, *di, *dv;
  MUTY *mu, *y;
  int *d_bi, *d_bv, *d_di, *d_dv;
  MUTY *d_mu, *d_y;
  struct type_f *d_f;
  int dbg=0;
  long maxk=MUMAX;
  struct type_f f;
  double t1, dt;
  
  if(argc>1) dbg=atoi(argv[1]);
  if(argc>2) maxk=atoi(argv[2]);
  
  // read mcc
  
    scanf("%d %d %d\n", &m, &n, &mm);
if(dbg>0)printf("m=%d n=%d mm=%d\n", m, n, mm);
  
  bi=(int *)malloc(MATRIX_SIZE(mm,n,int));
  bv=(int *)malloc(MATRIX_SIZE(mm,n,int));
  dv=(int *)malloc(MATRIX_SIZE(mm,n,int));
  di=(int *)malloc(MATRIX_SIZE(mm,n,int));
  mu=(MUTY *)malloc(VECTOR_SIZE(m,MUTY));
  if(dbg>2) y=(MUTY *)malloc(MATRIX_SIZE(mm,n,MUTY)); else y=(MUTY *)bi; // a trick to have no null
  if( bi==NULL || di==NULL || bv==NULL || dv==NULL || mu==NULL || y==NULL )
  {
    printf("*** error: not enough memory\n");
    exit(3);
  }
  
  read_matr_int(bi,mm,n);
if(dbg>2){
printf("bi:\n");
print_matr_int(bi,mm,n);}
  read_matr_int(bv,mm,n);
if(dbg>2){
printf("bv:\n");
print_matr_int(bv,mm,n);}

  read_matr_int(di,mm,n);
if(dbg>2){
printf("di:\n");
print_matr_int(di,mm,n);}
  read_matr_int(dv,mm,n);
if(dbg>2){
printf("dv:\n");
print_matr_int(dv,mm,n);}

#ifdef _LONG_MU_
  read_vect_long(mu,m);
if(dbg>0){
  printf("initial mu:\n");
  print_vect_long(mu,m);
}
#else
  read_vect_int(mu,m);
if(dbg>0){
  printf("initial mu:\n");
  print_vect_int(mu,m);
}
#endif
 
  // allocate device memory & copy to device
   
  CHECK(cudaSetDevice(0));

  int supportsCoopLaunch = 0;
  cudaDeviceGetAttribute(&supportsCoopLaunch, cudaDevAttrCooperativeLaunch, 0);

  if(!supportsCoopLaunch)
  {
    printf("*** no supportsCoopLaunch\n");
    exit(2);
  }

if(dbg>1) {
  cudaDeviceProp deviceProp;
  cudaGetDeviceProperties(&deviceProp, 0);
  printf("multiprocessors: %d\n",deviceProp.multiProcessorCount);
}

  CHECK(cudaMalloc((int**)&d_bi, MATRIX_SIZE(mm,n, int)));
  CHECK(cudaMalloc((int**)&d_bv, MATRIX_SIZE(mm,n, int)));
  CHECK(cudaMalloc((int**)&d_di, MATRIX_SIZE(mm,n, int)));
  CHECK(cudaMalloc((int**)&d_dv, MATRIX_SIZE(mm,n, int)));
  CHECK(cudaMalloc((MUTY**)&d_mu, VECTOR_SIZE(m, MUTY)));
  CHECK(cudaMalloc((MUTY**)&d_y, MATRIX_SIZE(mm,n, MUTY)));
  CHECK(cudaMalloc((struct type_f**)&d_f, sizeof(struct type_f) ));
  
  CHECK(cudaMemcpy(d_bi, bi, MATRIX_SIZE(mm,n, int), cudaMemcpyHostToDevice));
  CHECK(cudaMemcpy(d_bv, bv, MATRIX_SIZE(mm,n, int), cudaMemcpyHostToDevice));
  CHECK(cudaMemcpy(d_di, di, MATRIX_SIZE(mm,n, int), cudaMemcpyHostToDevice));
  CHECK(cudaMemcpy(d_dv, dv, MATRIX_SIZE(mm,n, int), cudaMemcpyHostToDevice));
  CHECK(cudaMemcpy(d_mu, mu, VECTOR_SIZE(m, MUTY), cudaMemcpyHostToDevice));
  
  // define device grid & block
    
  dim3 block (mm);
  dim3 grid  (n);

  void* kernelArgs[] = { (void*)&m,(void*)& n,(void*)& mm,(void*)& d_bi,(void*)& d_bv,(void*)& d_di,(void*)& d_dv,(void*)& d_mu,(void*)& d_y,(void*)& d_f,(void*)& maxk,(void*)& dbg };

  // run sn on device
//  printf("max_cooperative_grid_blocks(mm)=%d\n",run_sn.max_cooperative_grid_blocks(mm));
  
  t1=seconds();
  //run_sn<<<grid, block>>>(m, n, mm, d_bi, d_bv, d_di, d_dv, d_mu, d_y, d_f, maxk, dbg);
   cudaLaunchCooperativeKernel((void*)run_sn, grid, block, kernelArgs);
   CHECK(cudaDeviceSynchronize());
  dt=seconds()-t1;
  CHECK(cudaGetLastError());

  CHECK(cudaMemcpy(&f, d_f, sizeof(struct type_f), cudaMemcpyDeviceToHost));

if(dbg>1){
  printf("*** step: %ld, transition %d fired in %ld copies\n", f.k, f.t, (long)f.c);    
}

  // copy from device and print resulting marking
if(dbg>0){      
  CHECK(cudaMemcpy(mu, d_mu, VECTOR_SIZE(m,MUTY), cudaMemcpyDeviceToHost));  
  printf("final mu:\n");
  #ifdef _LONG_MU_  
    print_vect_long(mu,m);
  #else
    print_vect_int(mu,m);
  #endif
}

if(dbg>2){
  CHECK(cudaMemcpy(y, d_y, MATRIX_SIZE(mm,n, MUTY), cudaMemcpyDeviceToHost));
  printf("y:\n");
  #ifdef _LONG_MU_                              
    print_matr_long(y,mm,n);
  #else
    print_matr_int(y,mm,n);
  #endif 
}
  
  printf("--- it took %ld steps, time %f s. ---\n",f.k,dt);
  
  // free memory of device and host
  
  CHECK(cudaFree(d_bi));
  CHECK(cudaFree(d_bv));
  CHECK(cudaFree(d_di));
  CHECK(cudaFree(d_dv));
  CHECK(cudaFree(d_mu));
  CHECK(cudaFree(d_y));
  
  free(bi);
  free(bv);
  free(di); 
  free(dv); 
  free(mu); 
  if(dbg>2) free(y);

} // end of main

// daze@acm.org

