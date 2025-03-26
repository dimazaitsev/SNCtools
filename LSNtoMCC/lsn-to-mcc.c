// Convertor of LSN to MCC
//
// MSN: Sleptsov Net Raw Matrix File Format
// MCC: Sleptsov Net Matrix with Condensed Columns Format
//
// Compile: gcc -o lsn-to-mcc lsn-to-mcc.c
//
// Run: ./lsn-to-mcc lsn_file > mcc_file

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define MATRIX_SIZE(d1,d2,t) ((d1)*(d2)*(sizeof(t)))
#define VECTOR_SIZE(d1,t)    ((d1)*(sizeof(t)))

#define MOFF(i,j,d1,d2) ((d2)*(i)+(j))
#define MELT(x,i,j,d1,d2) (*((x)+MOFF(i,j,d1,d2)))

#define zmax(x,y) (((x)>(y))?(x):(y))
#define zmin(x,y) (((x)<(y))?(x):(y))

#define BUF_LEN 256

void zero_vect(int *x,int m)
{
  memset(x,0,VECTOR_SIZE(m,int));
}

void zero_matr(int *x,int m,int n)
{
  memset(x,0,MATRIX_SIZE(m,n,int));
}

int read_matr(int *x,int m,int n)
{
  int i,j;
  for(i=0;i<m;i++)
  {
    for(j=0;j<n;j++)
      scanf("%d",&MELT(x,i,j,m,n));
    //scanf("\n");
  }
}

int read_vect(int *x,int m)
{
  int i;
  for(i=0;i<m;i++)
  {
    scanf("%d",x+i);
  }
}

void cpy_matr(int *x,int m,int n,int*x1)
{
  memcpy(x1,x,MATRIX_SIZE(m,n,int));
}

void print_matr(int *x,int m,int n)
{
  int i,j;
  for(i=0;i<m;i++)
  {
    for(j=0;j<n;j++)
      printf("%2d ",MELT(x,i,j,m,n));
    printf("\n");
  }
}

void print_matr_perm(int * x, int m, int n, int* t_perm)
{
  int i,j;
  for(i=0;i<m;i++)
  {
    for(j=0;j<n;j++)
      printf("%2d ",MELT(x,i,t_perm[j],m,n));
    printf("\n");
  }
}

void print_vect(int *x,int m)
{
  int i;
  for(i=0;i<m;i++)
  {
    printf("%d ",x[i]);
  }
  printf("\n");
}

int print_mu(int *mu,int m)
{
  int i,p,v;
  for(i=0;i<m;i++)
  {
    v=mu[i];
    if(v) printf("%d %d\n",i+1,v);
  }
}

int mmax(int *a, int n){
  int mi=0,i;
  for(i=0;i<n;i++)
    if(a[i]>a[mi])mi=i;
  return a[mi];
}

// maximal number of nonzero elements over columns

int max_nz_col(int *x, int m, int n)
{
   int p, t, mmc, mm=0;
   
   for(t=0; t<n; t++)
   {
     mmc=0;
     for(p=0; p<m; p++)
     {
       if( MELT(x,p,t,m,n) != 0 ) mmc++;
     }
     mm=zmax(mm,mmc);
   }
   
   return( mm );
} // end of max_nz_col


int belong_to(int *v, int next, int x) // x belongs to v
{
  int t;
  for(t=0; t<next; t++)
    if( v[t] == x ) return (1);
  return(0);
} // end of belong_to


// compose permutation to sort transitions by priorities
//find_t_perm_mcc(n,rr_index,rr_value,nnr,rc_index,rc_value,nnc,t_perm);
void find_t_perm_mcc(int n, int * rr_i, int *rr_v, int nnr, int *rc_i, int *rc_v, int nnc, int *t_perm)
{
  int t,tt,tr,tc=0,ti,dbg=0;
  int ng,ts,tf;
  
  // find maximal transitions
  
  for(tt=0; tt<n; tt++)
  {
    ng=0;
    for(ti=0; ti<nnr; ti++)
    {
      if( MELT(rr_v,ti,tt,nnr,n) != 0 ) ng++;
    }
    if( ng==0 )
    {
      t_perm[tc++]=tt;
    }
  }

  ts=0;
  tf=tc;

if(dbg>1) printf("first layer ts=%d tf=%d tc=%d\n",ts,tf,tc);    
  
  while( tc < n )
  {
    for(tr=ts; tr<tf; tr++)
    {
      t=t_perm[tr];
if(dbg>1) printf("included tr=%d t=%d\n",tr,t);
      for(ti=0; ti<nnc; ti++)
      {
        if( MELT(rc_v,t,ti,n,nnc) != 0 )
        {
          tt=MELT(rc_i,t,ti,n,nnc);
if(dbg>1) printf("included ti=%d tt=%d\n",ti,tt);
          if( !belong_to(t_perm,tc,tt) ){
if(dbg>1) printf("included tt=%d\n",tt);
            t_perm[tc++]=tt;
          }
        }
      }
    }
if(dbg>1) printf("next layer ts=%d tf=%d tc=%d\n",ts,tf,tc);    
    ts=tf;
    tf=tc;
  }
  
} // end of find_t_perm



int main(int argc, char * argv[])
{
  int m, n, k, l, nst, dbg=0;
  int bmmm, dmmm, mm, nnr, nnc, i, v1, v2, w, p, t, tt;
  int *bmm, *dmm, *rcnn, *rrnn, *mu;
  int *b_index, *b_value, *d_index, *d_value, *rc_index, *rc_value, *rr_index, *rr_value;
  int *t_perm;
  char buf[BUF_LEN+1];
  FILE *f;
  
  if(argc>1) f = fopen(argv[1],"r");
  else {
  	printf("USAGE: lsn-to-mcc lsn_file\n"); 
	exit(2);
  }
  if( f==NULL ){
    printf("*** error opening file\n");
    return 3;
  }
  
  
  //if(argc>1)dbg=atoi(argv[1]);
  
  // read header
  
  do {
    fgets(buf,BUF_LEN,f);
  } while(buf[0]==';');
  sscanf(buf,"%d %d %d %d %d", &m, &n, &k, &l, &nst);
  
if(dbg>0){
	printf("buf: %s\n",buf);
	printf("m=%d n=%d k=%d l=%d nst=%d\n", m, n, k, l, nst);
}

  
  if(nst!=0){
    printf("*** supposedly HSN, not LSN\n");
    return 2;
  }
  
  bmm=malloc(VECTOR_SIZE(n,int));
  dmm=malloc(VECTOR_SIZE(n,int));
  rcnn=malloc(VECTOR_SIZE(n,int));
  rrnn=malloc(VECTOR_SIZE(n,int));
  if(bmm==NULL || dmm==NULL || rcnn==NULL || rrnn==NULL ){
    printf("*** no memory for bmm,dmm,rnn\n");
    return 3;
  }
  
  zero_vect(bmm,n);
  zero_vect(dmm,n);
  zero_vect(rrnn,n);
  zero_vect(rcnn,n);
  
  // read LSN and compute sizes
   
  for(i=0;i<k;i++)
  {
    do {
      fgets(buf,BUF_LEN,f);
    } while(buf[0]==';');
    sscanf(buf,"%d %d %d",&v1,&v2,&w);
if(dbg>2)printf("%d %d %d\n",v1,v2,w);
    if(v1 < 0 && v2 < 0) //pr
    { 
      t=-v1-1; tt=-v2-1;
      (rcnn[t])++;
      (rrnn[tt])++;
    }
    else if(v1 > 0 && v2 > 0) // p->t
    {
      p=v1-1; t=v2-1;
      (bmm[t])++;
    }
    else if(v1 < 0 && v2 > 0) // t->p
    {
      p=-v1-1; t=v2-1;
      (dmm[t])++;
    } 
  }

if(dbg>0){
  printf("bmm:\n");
  print_vect(bmm,n);
  printf("\ndmm:\n");
  print_vect(dmm,n);
  printf("\nrrnn:\n");
  print_vect(rrnn,n);
  printf("\nrcnn:\n");
  print_vect(rcnn,n);
}  

  bmmm=mmax(bmm,n);
  dmmm=mmax(dmm,n);
  mm=zmax(bmmm,dmmm);
  nnr=mmax(rrnn,n);
  nnc=mmax(rcnn,n);
  
if(dbg>0){
  printf("bmmm=%d,dmmm=%d,mm=%d,nnr=%d, nnc=%d\n",bmmm,dmmm,mm,nnr,nnc);
}  
   
  // allocate
  
  b_index=malloc(MATRIX_SIZE(mm,n,int));
  b_value=malloc(MATRIX_SIZE(mm,n,int));
  d_index=malloc(MATRIX_SIZE(mm,n,int));
  d_value=malloc(MATRIX_SIZE(mm,n,int));
  rr_index=malloc(MATRIX_SIZE(nnr,n,int)); // column direct access
  rr_value=malloc(MATRIX_SIZE(nnr,n,int));
  rc_index=malloc(MATRIX_SIZE(n,nnc,int)); // row direct access
  rc_value=malloc(MATRIX_SIZE(n,nnc,int));
  mu=malloc(VECTOR_SIZE(m,int));
  t_perm=malloc(VECTOR_SIZE(n,int));
  if(b_index==NULL || b_value==NULL || d_index==NULL || d_value==NULL || 
     rr_index==NULL || rr_value==NULL || rr_index==NULL || rr_value==NULL ||
     mu==NULL || t_perm==NULL ){
    printf("*** no memory for b_index,b_value...\n");
    return 3;
  }
  
  zero_matr(b_index,mm,n);
  zero_matr(b_value,mm,n);
  zero_matr(d_index,mm,n);
  zero_matr(d_value,mm,n);
  zero_matr(rr_index,nnr,n);
  zero_matr(rr_value,nnr,n);
  zero_matr(rc_index,n,nnc);
  zero_matr(rc_value,n,nnc);
  zero_vect(mu,m);
    
  // read into MCC
  
  rewind(f);
  
  // read header
  do {
    fgets(buf,BUF_LEN,f);
  } while(buf[0]==';');
  sscanf(buf,"%d %d %d %d %d\n", &m, &n, &k, &l, &nst);

if(dbg>0){
	printf("buf: %s\n",buf);
	printf("m=%d n=%d k=%d l=%d nst=%d\n", m, n, k, l, nst);
}
  
  zero_vect(bmm,n);
  zero_vect(dmm,n);
  zero_vect(rrnn,n);
  zero_vect(rcnn,n);
  
  // read arcs
  
  for(i=0;i<k;i++)
  {
    do {
      fgets(buf,BUF_LEN,f);
    } while(buf[0]==';');
    sscanf(buf,"%d %d %d",&v1,&v2,&w);
if(dbg>2)printf("arc: %d %d %d\n",v1,v2,w);
    if(v1 < 0 && v2 < 0) //pr
    { 

      t=-v1-1; tt=-v2-1;
      MELT(rc_value,t,rcnn[t],n,nnc)=1;
      MELT(rc_index,t,rcnn[t],n,nnc)=tt;
      (rcnn[t])++;
      
      MELT(rr_value,rrnn[tt],tt,nnr,n)=1;
      MELT(rr_index,rrnn[tt],tt,nnr,n)=t;
      (rrnn[tt])++;
    }
    else if(v1 > 0 && v2 > 0) // p->t
    {
      p=v1-1; t=v2-1;
      MELT(b_value,bmm[t],t,mm,n)=w;
      MELT(b_index,bmm[t],t,mm,n)=p;
      (bmm[t])++;
     }
    else if(v1 < 0 && v2 > 0) // t->p
    {
      p=-v1-1; t=v2-1;
      MELT(d_value,dmm[t],t,mm,n)=w;
      MELT(d_index,dmm[t],t,mm,n)=p;
      (dmm[t])++;
    }
  }
  
  // read marking
  for(i=0;i<l;i++){
  	do {
  	  fgets(buf,BUF_LEN,f);
    } while(buf[0]==';');
    sscanf(buf, "%d %d",&v1,&v2);
if(dbg>2)printf("mu: %d %d\n",v1,v2);
    p=v1-1;
    mu[p]=v2;
  }
  
if(dbg>1){
  printf("rr_index:\n");
  print_matr(rr_index,nnr,n);
  printf("rr_value:\n");
  print_matr(rr_value,nnr,n);
  printf("rc_index:\n");
  print_matr(rc_index,n,nnc);
  printf("rc_value:\n");
  print_matr(rc_value,n,nnc);
}
  
  // order trs on their priorities

  find_t_perm_mcc(n,rr_index,rr_value,nnr,rc_index,rc_value,nnc,t_perm);
  //for(i=0;i<n;i++) t_perm[i]=i;
  
  // output MCC

  printf("%d %d %d\n", m, n, mm);
  print_matr_perm(b_index,mm,n,t_perm); printf("\n");
  print_matr_perm(b_value,mm,n,t_perm); printf("\n");
  print_matr_perm(d_index,mm,n,t_perm); printf("\n");
  print_matr_perm(d_value,mm,n,t_perm); printf("\n");
  print_vect(mu,m); 
  printf("\n# permutation of transitions:\n");
  print_vect(t_perm,n); 
  
// free memory  

  free(bmm); free(dmm); free(rcnn); free(rrnn);
  free(b_index);
  free(b_value);
  free(d_index);
  free(d_value);
  free(rr_index);
  free(rr_value);
  free(rc_index);
  free(rc_value);
  free(t_perm);
  free(mu); 

}

//  2024 daze@acm.org
