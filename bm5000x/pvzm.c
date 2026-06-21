// Run PTN as SVS (PVZ) machine (https://doi.org/10.1080/17445760.2026.2615010)
// using processes created by fork()
// side effect - semaphores remain in kernel - try to get rid off 'em :-)

// v3 - sparse matrix specification of all sops, alpha

// $ gcc -O3 -o pvzm pvzm.c 
// $ ./pvzm
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
#include <signal.h>

#include <limits.h>
#include <sys/time.h>

double seconds()
{
    struct timeval tp;
    struct timezone tzp;
    int i = gettimeofday(&tp, &tzp);
    return ((double)tp.tv_sec + (double)tp.tv_usec * 1.e-6);
}

// include PTN SVS (PVZ data) .h file name here
#include "gm16x16-b6.h"
//#include "gm32x32-b6.h"

int pls_sem;
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

void *transition (int t)
{

  for(;;) {

    // Group P/V operation on places (semaphores)

    if (semop (pls_sem, sops+sops_idx[t], sops_idx[t+1]-sops_idx[t]) == -1) {
      sprintf(err_buf,"semop: Group P/V/Z operation on places t_pls[%d]",t);
      perror (err_buf); exit (1);
    }

    if (deb==1) printf("t%d ", t ); else
    if (deb>1) prn_mu_t(t);
    fflush(stdout);

  } // for(;;)

} // end of transition()

int main (int argc, char **argv)
{
    pid_t pid, pid_trs [N];
    int t, tt, r;
    double t1, dt;
    
    if(argc>1) deb=atoi(argv[1]);

    //  semafore array as the current place marking
    // create a set of M semaphores
    if ((pls_sem = semget (IPC_PRIVATE, M, 0660 | IPC_CREAT)) == -1) {
        perror ("semget"); exit (1);
    }
    // assign initial marking
    if (semctl (pls_sem, 0, SETALL, s) == -1) {
        perror ("semctl SETALL"); exit (1);
    }
    printf("init  "); prn_mu();

    // create N transition processes
    for (t = 0; t < N; t++) {
       tt=t;
       pid = fork();
       if (pid < 0) { // error
          fprintf (stderr, "Error = %d (%s)\n", r, strerror (r)); exit (1);
       } else if (pid > 0) { // parent process
         pid_trs[tt] = pid;
         // printf("fork child %d\n", (int) pid_trs [t]);
       } else break; // pid == 0, child process
    } // for
    
    if (pid == 0) { // a child process
      transition(tt);
    }
    
    // a parent process
    t1=seconds();
    // check termination condition
    if (semop (pls_sem, fin_sops, fin_sops_idx[1]-fin_sops_idx[0]) == -1) {
      perror ("semop: P0 on fin_pls"); exit (1);
    }
    dt=seconds()-t1;
    printf("%f ",dt);
    
    sleep(1); 
    
    printf("\n" );
    printf("final   "); prn_mu();
    
    // terminate transition processes
    for (t = 0; t < N; t++) {
        // printf("kill %d\n", (int) pid_trs [t]);
        if ((r = kill (pid_trs [t],SIGTERM)) != 0) {
          fprintf (stderr, "Error = %d (%s)\n", r, strerror(errno)); exit (1);
        }
    }
    
    // delete set of semaphores
    if (semctl (pls_sem, 0, IPC_RMID) == -1) {
        perror ("semctl IPC_RMID"); exit (1);
    }

    exit (0);
} // end of main()

