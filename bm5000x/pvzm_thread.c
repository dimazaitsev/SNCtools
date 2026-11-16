// Run PTN as PVZ machine as described in open access papers:
// https://doi.org/10.1080/17445760.2026.2615010 obsoleted by
// 
// using System V semaphores with semop() and threads created by pthread()
// Please mind, data is inside the app, inserted via #include;
// it requires recompiling the app for new data (PTN)
// v.3.1 alpha - sparse matrix specification of all sops
// @ 2026 Dmitry Zaitsev: zsoftua@yahoo.com

// Compile-build and run
// Edit the pvzm_data.h filename in pvzm_thread.c
// $ gcc -O3 -o pvzm_thread pvzm_thread.c -pthread
// $ ./pvzm_thread 
// or
// $ ./pvzm_thread 1
// parameter - output detail level

// Useful commands to handle semaphores:
// $ ipcs -s             % show semaphores
// $ ipcs -s -i <semid>  % show semaphore values and attache processes
// $ ipcrm -s <semid>    % delete by semid

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <limits.h>
#include <sys/time.h>


// *********** DATA IS INSERTED HERE VIA #include "pvzm_data.h" ***********
// include PTN, saved as PVZ machine data, .h file name here
// please uncomment only one #include or use your data
#include "gm16x16-b6.h"
//#include "gm32x32-b6.h"
//
// for learning try
//#include "add.h"
//#include "mul.h"

double seconds()
{
    struct timeval tp;
    struct timezone tzp;
    int i = gettimeofday(&tp, &tzp);
    return ((double)tp.tv_sec + (double)tp.tv_usec * 1.e-6);
}

int pls_sem;
int t_arg[N]; // memory for thread arguments
char err_buf[256];
int deb=0;

void prn_mu() { // print marking (semaphores)
    // Get marking
    if (semctl (pls_sem, 0, GETALL, s) == -1) {
        perror ("semctl GETALL mu"); exit (1);
    }
    printf("mu: ");
    for(int p=0;p<M;p++) printf("%d ",s[p]);
    printf("\n");
    fflush(stdout);
} // end of prn_mu()

void prn_mu_t(int t) { // print marking (semaphores)
    // Get marking
    if (semctl (pls_sem, 0, GETALL, s) == -1) {
        perror ("semctl GETALL mu"); exit (1);
    }
    printf("t%d => mu: ", t);
    for(int p=0;p<M;p++) printf("%d ",s[p]);
    printf("\n");
    fflush(stdout);
} // end of prn_mu_t()

void *transition (void *arg)
{
  int t = *((int *) arg);

  for(;;) {

    // Group P/V/Z operation on places (semaphores)

    if (semop (pls_sem, sops+sops_idx[t], sops_idx[t+1]-sops_idx[t]) == -1) {
      if(errno == EIDRM) return NULL; // semaphores deleted by semctl (pls_sem, 0, IPC_RMID) in main()
      else {
        sprintf(err_buf,"semop: Group P/V/Z operation on places t_pls[%d]",t);
        perror (err_buf); exit (1);
      }
    }

    if (deb == 2) printf("t%d ", t ); else
    if (deb > 2) prn_mu_t(t);
    fflush(stdout);

  } // for(;;)

} // end of transition()

int main (int argc, char **argv)
{
    pthread_t tid_trs [N];
    int t, r;
    double t1, dt;
    
    if(argc>1) deb=atoi(argv[1]);

    //  semafore array as the current place marking
    // create a set of M semaphores
    if ((pls_sem = semget (IPC_PRIVATE, M, 0660 | IPC_CREAT)) == -1) {
        perror ("semget"); exit (1);
    }
    // assign initial marking
    if (semctl (pls_sem, 0, SETALL, s) == -1) {
        perror ("semctl, SETALL"); exit (1);
    }
    if(deb>0) { printf("init "); prn_mu(); }

    // create N transition threads
    for (t = 0; t < N; t++) {
        t_arg[t] = t;
        if ((r = pthread_create (&tid_trs [t], NULL, transition, (void *) &t_arg[t] )) != 0) {
            perror("pthread_create"); exit (1);
        }
    }
    
    t1=seconds();
    // check termination condition
    if (semop (pls_sem, fin_sops, fin_sops_idx[1]-fin_sops_idx[0]) == -1) {
      perror ("semop, fin_sops"); exit (1);
    }
    dt=seconds()-t1;
    
    sleep(1); // for correct completion actions timing
    
    if(deb>0) { printf("\nfin  "); prn_mu(); }
    
    printf("%f ",dt);
    if(deb>0) printf("\n");
          
    // delete set of semaphores
    if (semctl (pls_sem, 0, IPC_RMID) == -1) {
        perror ("semctl, IPC_RMID"); exit (1);
    }
    
    // join created threads with the main thread for correct completion
    for(t=0; t < N; t++ ) {
      r = pthread_join( tid_trs[t], NULL );
      if( r != 0 ) {
        perror ("pthread_join"); exit (1);
      }
    }

    exit (0);
} // end of main()

// @ 2026 Dmitry Zaitsev: zsoftua@yahoo.com 

