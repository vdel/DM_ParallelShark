/* communication.c */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <mpi.h>


#include "fish.h"
#include "communication.h"


/*
 * neigh_right
 *
 * Retourne le numéro du voisin de droite 
 *
 */
int
neigh_right (void)
{
  return (unsigned int)((myid + 1 + numprocs)%numprocs);  
}

/*
 * neigh_left
 *
 * Retourne le numéro du voisin de gauche 
 *
 */
int
neigh_left (void)
{
  return (unsigned int)((myid - 1 + numprocs)%numprocs);  
}


/*
 * comm_send
 *
 * Envoie les informations à where sur le poisson (x,y)
 *
 */
struct comm_t *
comm_send (char where, int x, int y, int flag)
{
  struct comm_t comm;
  comm.flag = flag;
  comm.line = y; 
  comm.fish = *(local_fish_at(x,y)); 
  comm.temoin = ++id_packet;
  MPI_Send(&comm, sizeof (struct comm_t), MPI_CHAR, (where==TO_RIGHT)?neigh_right():neigh_left(), where, MPI_COMM_WORLD);
#ifdef _DEBUG
  printf("[%d] Envoie vers %s(%d), ligne %d\n", myid, (where==TO_RIGHT)?"droite":"gauche",
     (where==TO_RIGHT)?neigh_right():neigh_left(), comm.line);fflush(stdout);
#endif
  memcpy(last_sent, &comm, sizeof(struct comm_t));
  return NULL;
}

/*
 * comm_send_backup
 *
 * Envoie les informations à where sur le poisson sauvegardé de la colonne de gauche 
 *
 */
void
comm_send_backup (char where, int y, int flag)
{
  struct comm_t comm;
  comm.flag = flag;
  comm.line = y; 
  comm.fish.type = column_left[y];
  comm.fish.age = 0;
  comm.fish.age_before_starvation = 0;
  comm.fish.not_to_treat = 0;
  comm.temoin = ++id_packet;
#ifdef _DEBUG
  printf("[%d] Envoie sauvegarde vers %s(%d), ligne %d\n", myid, (where==TO_RIGHT)?"droite":"gauche", 
  (where==TO_RIGHT)?neigh_right():neigh_left(),
comm.line);fflush(stdout);
#endif
   MPI_Send(&comm, sizeof (struct comm_t), MPI_CHAR, (where==TO_RIGHT)?neigh_right():neigh_left(), where, MPI_COMM_WORLD);
}

