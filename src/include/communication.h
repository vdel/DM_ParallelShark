#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include "fish.h"
#include <mpi.h>

#define COMM_SIZE 4096
#define NO_COND -1
#define NO_ID -1

#define MAX_ATTEMPT 500
#define TIME_TO_WAIT 10

#define FROM_LEFT 1
#define FROM_RIGHT 0

#define TO_LEFT 0
#define TO_RIGHT 1

struct comm_t
{ 
  char flag;
  struct fish_struct_t fish;
  int line;
  int temoin;
};

struct comm_stack
{
  char where; /* 0 = droite, 1 = gauche */
  struct comm_t *comm;
  struct comm_stack *next;
  struct comm_stack *prev;
};

extern struct comm_stack base_stack;

struct fish_tab_t
{
  int x,y;
  int type;
};

#define PROBLEM 10
#define GO_THROUGH  1
#define NOT_THROUGH 0
#define CONFLICT    2
#define LETSGO 5
#define TERMINATION 3

/* Etat d'urgence */
#define TAKE_HIS_HIGH_FIVE          8
#define GIVE_HIM_YOUR_RIGHT_FIVE   16
#define TAKE_HIS_LEFT_FIVE         32
#define GIVE_HIM_YOUR_LEFT_FIVE    64
#define ONYGO                     128
#define OKIHAVEONE                256
#define IDONTHAVEONE              512
#define GIVE_FISH_LIST           1024
#define LOAD_LOCK 125
#define LOAD_UNLOCK 115

extern MPI_Request *req_left;
extern MPI_Request *req_right;
extern int id_packet;
extern struct comm_t *comm_left;

extern struct comm_t *comm_right;
extern struct comm_t *last_sent;

int neigh_right (void);
int neigh_left (void);
int request_solve (int stop_line, char where, struct comm_t *);
void waiting_loop (char, int, struct comm_t *);
void comm_send2 (int, int, int, int, int);
struct comm_t * comm_send (char, int, int, int);
void comm_send_backup (char where, int y, int flag);
void handle_request (struct comm_t *, int, char);
void insert_in_stack (struct comm_t *, char);
void delete_from_stack (struct comm_stack *);
void clear_stack();
int stack_exists (int, char);


#endif /* COMMUNICATION_H */
