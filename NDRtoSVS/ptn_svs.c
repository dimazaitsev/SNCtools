// Run TPN as LSVS machine

// $ gcc tpn_lsvs.c -lpthread -o tpn_lsvs
// $ ./tpn_lsvs
// $ cat /proc/sysvipc/sem
// $ ipcs -s -i 0
// $ cat /proc/sys/kernel/sem
// $ ipcrm -s <semid>    delete by semid

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


// include PTN LSVS .h file name here
#include "mul.h"

int pls_sem;
char err_buf[256];
int deb=0;

void prn_mu() {
    // Get marking
    if (semctl (pls_sem, 0, GETALL, mu) == -1) {
        perror ("semctl GETALL mu"); exit (1);
    }
    printf("mu: ");
    for(int p=0;p<M;p++) printf("%d ",mu[p]);
    printf("\n");
    fflush(stdout);
} // end of prn_mu()

void *transition (void *arg)
{
  int t = *((int *) arg);

  for(;;) {

    // Group P/V operation on places

    if (semop (pls_sem, t_pls[t], t_pls_num[t]) == -1) {
      sprintf(err_buf,"semop: Group P/V/Z operation on places t_pls[%d]",t);
      perror (err_buf); exit (1);
    }

    if (deb>0) printf("t%d ", t );
    if (deb>1) prn_mu();
    fflush(stdout);

  } // for(;;)

} // end of transition()

int main (int argc, char **argv)
{
    pthread_t tid_trs [N];
    int t, r;
    
    if(argc>1) deb=atoi(argv[1]);

    //  semafore array as the current place marking
    // create a set of M semaphores
    if ((pls_sem = semget (IPC_PRIVATE, M, 0660 | IPC_CREAT)) == -1) {
        perror ("semget"); exit (1);
    }
    // assign initial marking
    if (semctl (pls_sem, 0, SETALL, mu) == -1) {
        perror ("semctl SETALL"); exit (1);
    }
    printf("initial "); prn_mu();

    // create N transition threads
    for (t = 0; t < N; t++) {
        t_arg[t] = t;
        if ((r = pthread_create (&tid_trs [t], NULL, transition, (void *) &t_arg[t] )) != 0) {
            fprintf (stderr, "Error = %d (%s)\n", r, strerror (r)); exit (1);
        }
    }

    // check termination condition
    if (semop (pls_sem, fin_pls, fin_pls_num) == -1) {
      perror ("semop: P0 on fin_pls"); exit (1);
    }
    printf("\n" );
    printf("final   "); prn_mu();
    
    // cancel threads
    for (t = 0; t < N; t++) {
      if ((r = pthread_cancel (tid_trs [t])) != 0) {
        fprintf (stderr, "Error = %d (%s)\n", r, strerror (r)); exit (1);
      }
    }
    
    // delete set of semaphores
    if (semctl (pls_sem, 0, IPC_RMID) == -1) {
        perror ("semctl IPC_RMID"); exit (1);
    }

    exit (0);
} // end of main()

