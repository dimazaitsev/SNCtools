// CPU WN VM, input MCC, no priorities, WN linear approximation, alpha
// Compile: gcc -O3 -o wn-bm wn-bm.c
// Run:     ./wn-bm < net.mcc
// @ daze@acm.org

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/time.h>

// data length definitions

//#define _LONG_MU_

#ifdef _LONG_MU_

#define MUTY long
#define MU_MAX LONG_MAX

#else

#define MUTY int
#define MU_MAX INT_MAX

#endif

// end of data length definitions

#define MATRIX_SIZE(d1,d2,t) ((d1)*(d2)*(sizeof(t)))
#define VECTOR_SIZE(d1,t)    ((d1)*(sizeof(t)))

#define MOFF(i,j,d1,d2) ((d2)*(i)+(j))
#define MELT(x,i,j,d1,d2) (*((x)+MOFF(i,j,d1,d2)))

#define zmax(x,y) (((x)>(y))?(x):(y))
#define zmin(x,y) (((x)<(y))?(x):(y))

// arc firing multiplicity macros for regular (w>0) and inhibitor (w=-1) arcs; MU_MAX as infinity

#define arc_firing(pi,t) ((MELT(bv,(pi),(t),mm,n)>0)? mu[MELT(bi,(pi),(t),mm,n)] / MELT(bv,(pi),(t),mm,n) : (MELT(bv,(pi),(t),mm,n)<0)? ((mu[MELT(bi,(pi),(t),mm,n)]>0)? 0: MU_MAX): MU_MAX)

double seconds()
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

void run_sn(int m, int n, int mm, int *bi, int *bv, int *di, int *dv, MUTY *mu, MUTY *y, 
            struct type_f *f, long maxk, int dbg)
{
  MUTY af;
  int pi, t, kk, pii, rollback;

  (f->k)=0;

  while( (maxk==-1) || ((f->k)<maxk) )
  {
    // transitions firing multiplicity

    for(t=0; t<n; t++) {
      af=arc_firing(0,t);   
    
        for(pi=1; pi<mm; pi++) {
          af=zmin(af,arc_firing(pi,t));             
        }

      y[t]=af;
    }
    
if(dbg>2){
  printf("y at %ld:\n",f->k);
  #ifdef _LONG_MU_                              
    print_vect_long(y,n);
  #else
    print_vect_int(y,n);
  #endif 
}
 
    // SWN rule sequential approximation until negative marking obtained, then rollback the current transition
    kk=0;
    rollback=0;
    for(t=0; t<n; t++) {
      if(y[t]>0) {
         (f->c)=y[t];
         (f->t)=t;

         // fire transition
 
         for(pi=0; pi<mm; pi++) // next_mu
         {
           if(MELT(bv,pi,f->t,mm,n)>0) mu[MELT(bi,pi,f->t,mm,n)]-=MELT(bv,pi,f->t,mm,n);

           if( mu[MELT(bi,pi,f->t,mm,n)] < 0 ) {
             rollback=1;
             pii=pi;
             break;
           }
           
         }

         if(rollback){
           for(pi=pii; pi>=0; pi++) {
             if(MELT(bv,pi,f->t,mm,n)>0) mu[MELT(bi,pi,f->t,mm,n)]+=MELT(bv,pi,f->t,mm,n);
           }
           break;
         }
           
         kk++;

if(dbg>1){
  printf("step %ld, fired %d in %ld copies mu:\n",f->k,f->t,(long)(f->c));
  #ifdef _LONG_MU_
    print_vect_long(mu,m);
  #else
    print_vect_int(mu,m);
  #endif
}
       } // if(y[t]>0)
     } // for(t=0; t<n; t++)
     
     // add positives (outgoing arcs)
     for(t=(f->t)-rollback; t>=0; t--) {
        if(y[t]>0) {
          for(pi=0; pi<mm; pi++) {
              if(MELT(dv,pi,t,mm,n)>0) mu[MELT(di,pi,t,mm,n)]+=MELT(dv,pi,t,mm,n);
          }
        }
     } // for(t=(f->t)-roolback; t>=0; t--) 


if(dbg>1){
  printf("step %d fired %d transitions, marking corrected mu:\n",(f->k),kk);
  #ifdef _LONG_MU_
    print_vect_long(mu,m);
  #else
    print_vect_int(mu,m);
  #endif
}
     
    if(kk==0) break;
    (f->k)++;

  } // end of while

} // end of run_sn


int main(int argc, char * argv[])
{
  int m, n, mm;
  int *bi, *bv, *di, *dv;
  MUTY *mu, *y;
  struct type_f *d_f;
  int dbg=0, maxk=-1;
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
  y=(MUTY *)malloc(VECTOR_SIZE(n,MUTY)); 
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
 
  t1=seconds();
  run_sn(m, n, mm, bi, bv, di, dv, mu, y, &f, maxk, dbg);
  dt=seconds()-t1;

if(dbg>1){
  printf("*** step: %ld, transition %d fired in %ld copies\n", f.k, f.t, (long)f.c);    
}

  // copy from device and print resulting marking
if(dbg>0){      
  printf("final mu:\n");
  #ifdef _LONG_MU_  
    print_vect_long(mu,m);
  #else
    print_vect_int(mu,m);
  #endif
}

  //printf("--- it took %ld steps, time %f s. ---\n",f.k,dt);
  printf("%f ",dt);
  
  // free memory of device and host
    
  free(bi);
  free(bv);
  free(di); 
  free(dv); 
  free(mu); 
  free(y);

} // end of main

// @ daze@acm.org

