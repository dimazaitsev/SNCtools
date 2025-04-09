#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/time.h>
#include <omp.h>

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
  int pi, t;
  int ipos;
  int ipos_min=MU_MAX;

  (f->k)=0;

  #pragma omp parallel private(t, pi)
  while( (maxk==-1) || ((f->k)<maxk) )
  {
    #pragma omp for
    for(t=0; t<n; t++) {
      af=arc_firing(0,t);
      #pragma omp simd reduction(min:af)
      #pragma omp unroll
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

    // choose firing transition
    (f->c)=0; (f->t)=0;

    for(t=0; t<n; t++) {
      if(y[t]>0) {
          (f->c)=y[t]; // firing multiplicity
          (f->t)=t;    // firing transition number
          break;
      }
    }



    // fire transition
    if((f->c)>0){
      #pragma omp for
      for(pi=0; pi<mm; pi++) // next_mu
      {
        if(MELT(bv,pi,f->t,mm,n)>0) mu[MELT(bi,pi,f->t,mm,n)]-=(f->c)*MELT(bv,pi,f->t,mm,n);
        if(MELT(dv,pi,f->t,mm,n)>0) mu[MELT(di,pi,f->t,mm,n)]+=(f->c)*MELT(dv,pi,f->t,mm,n);
      }
    } else break;

if(dbg>1){
  printf("step %ld, fired %d in %ld copies mu:\n",f->k,f->t,(long)(f->c));
  #ifdef _LONG_MU_
    print_vect_long(mu,m);
  #else
    print_vect_int(mu,m);
  #endif
}

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

  printf("--- it took %ld steps, time %f s. ---\n",f.k,dt);

  // free memory of device and host

  free(bi);
  free(bv);
  free(di);
  free(dv);
  free(mu);
  free(y);

} // end of main

// daze@acm.org
