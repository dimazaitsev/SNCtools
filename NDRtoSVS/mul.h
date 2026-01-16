// PTN in the form of SVS obtained from NDR

#define M 9 // number of places 
#define N 8 // number of transitions

unsigned short mu[] = {0,13,25,0,1,0,1,1,1}; // initial place marking

// incidental places of transitions - semaphore operations
// struct sembuf {
//   unsigned short sem_num; // which semaphore in the set
//   short sem_op; // operation
//   sem_flg; // behavior flags
// };
struct sembuf t0[] = { {0,0,0},{2,-1,0},{4,-1,0},{0,1,0} };
struct sembuf t1[] = { {1,-1,0},{4,0,0},{3,1,0},{5,1,0} };
struct sembuf t2[] = { {1,0,0},{4,0,0},{6,-1,0},{4,1,0} };
struct sembuf t3[] = { {5,-1,0},{6,0,0},{1,1,0} };
struct sembuf t4[] = { {0,-1,0},{5,0,0},{6,0,0},{6,1,0} };
struct sembuf t5[] = { {0,0,0},{2,0,0},{7,-1,0},{0,1,0} };
struct sembuf t6[] = { {1,-1,0},{7,0,0} };
struct sembuf t7[] = { {1,0,0},{7,0,0},{8,-1,0},{7,1,0} };
struct sembuf * t_pls[] = {t0,t1,t2,t3,t4,t5,t6,t7};
int t_pls_num [] = {4,4,4,3,4,4,2,4};
int t_arg[N]; // memory for thread arguments

// termination condition
struct sembuf fin_pls[] = { {8,0,0} };
int fin_pls_num = 1;

// end of PTN in the form of LSVS
// 0 p0
// 1 p1
// 2 p2
// 3 p3
// 4 p4
// 5 p5
// 6 p6
// 7 p7
// 8 p8
// 0 t0
// 1 t1
// 2 t2
// 3 t3
// 4 t4
// 5 t5
// 6 t6
// 7 t7
