// PTN in the form of SVS obtained from NDR

#define M 5 // number of places 
#define N 3 // number of transitions

unsigned short mu[] = {0,3,2,0,1}; // initial place marking

// incidental places of transitions - semaphore operations
// struct sembuf {
//   unsigned short sem_num; // which semaphore in the set
//   short sem_op; // operation
//   sem_flg; // behavior flags
// };
struct sembuf t0[] = { {0,0,0},{1,0,0},{2,0,0},{4,-1,0},{0,1,0} };
struct sembuf t1[] = { {0,0,0},{1,-1,0},{3,1,0} };
struct sembuf t2[] = { {0,0,0},{2,-1,0},{3,1,0} };
struct sembuf * t_pls[] = {t0,t1,t2};
int t_pls_num [] = {5,3,3};
int t_arg[N]; // memory for thread arguments

// termination condition
struct sembuf fin_pls[] = { {4,0,0} };
int fin_pls_num = 1;

// end of PTN in the form of LSVS
// 0 p0
// 1 p1
// 2 p2
// 3 p3
// 4 p4
// 0 t0
// 1 t1
// 2 t2
