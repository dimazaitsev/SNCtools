// PTN in the form of PVZM data obtained from NDR

// number of semaphores (places of PTN)
#define M 9
// number of processes (transitions of PTN)
#define N 8

// semaphores (places of PTN)
unsigned short s [] = {0,13,25,0,1,0,1,1,1}; // initial place marking

// (arcs of PTN)
struct sembuf sops [] = {
	 {0,0,0},{2,-1,0},{4,-1,0},{0,1,0},
	 {1,-1,0},{4,0,0},{3,1,0},{5,1,0},
	 {1,0,0},{4,0,0},{6,-1,0},{4,1,0},
	 {5,-1,0},{6,0,0},{1,1,0},
	 {0,-1,0},{5,0,0},{6,0,0},{6,1,0},
	 {0,0,0},{2,0,0},{7,-1,0},{0,1,0},
	 {1,-1,0},{7,0,0},
	 {1,0,0},{7,0,0},{8,-1,0},{7,1,0} };
// (arcs of transitions idx)
int sops_idx [] = { 0,4,8,12,15,19,23,25,29 };

// termination condition
struct sembuf fin_sops [] = { {8,0,0} };
int fin_sops_idx [] = { 0,1 };

// end of PTN in the form of PVZM data
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
