/* fish_utils */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <mpi.h>

#include "communication.h"
#include "fish.h"



struct comm_stack base_stack = { 0, NULL, NULL , NULL};



/*
 * num_fish
 *
 * Compte le nombre de poisson du type voulu dans la bande 
 *
 */

int 
num_fish (char type) 
{
  int x, y;
  int fishes = 0;


  for(y = 0; y < ocean_size; y++) 
    for(x=0; x<bande_width; x++) 
      if (fish_at(x,y) != NULL && fish_at(x,y)->type != NOTHING)        
      	if ((int)(fish_at(x,y)->type & type) == (int)fish_at(x,y)->type) 
          fishes++;
     
  return fishes;
}

/*
 * print_bande()
 * 
 * Permet l'affichage de la grille, si celle-ci est de petite taille.
 *
 */

void 
print_bande () 
{
  int x, y;
  int requins = 0;
  int sardines = 0;

  fprintf(stdout," ");  
  for (x = 0; x < bande_width + 2; x++) 
    fprintf(stdout, "-");
  fprintf(stdout,"\n");

  for(y = 0; y < ocean_size; y++) 
  {
    fprintf(stdout,"%d|",y%10);
    for(x=0; x<bande_width; x++) 
    {
      if (fish_at(x,y) != NULL) 
      {
      	if (fish_at(x,y)->type == SHARK) 
        {
	        fprintf(stdout,"\033[31mR\033[00m");
	        requins++;
	      }  
        else if (fish_at(x,y)->type == SARDINE) 
        {
      	  fprintf(stdout,"S");
      	  sardines++;
      	}
      else 
      	fprintf(stdout," ");
      } 
      else 
      	fprintf(stdout," ");
    }
    fprintf(stdout,"|\n");
  }

  fprintf(stdout," ");  
  for (x = 0; x < bande_width + 2; x++) 
    fprintf(stdout,"-");

  fprintf(stdout,"\n");
  fprintf(stdout,"%d requins et %d sardines.\n\n",requins,sardines);
}


/*
 * num_fish
 *
 * Compte le nombre de poisson du type voulu dans la bande 
 *
 */
void
waiting_loop (char where, int line, struct comm_t *comm2)
{
  int flag;
 
 struct comm_t *comm;
  struct comm_t *comm_sent = malloc(sizeof(struct comm_t));
  MPI_Request *req;

  int received = 0;
  int i = 0;


#ifdef _DEBUG
  printf("[%d] on attend de %s(%d) ligne %d\n", myid, (req==req_left)?"gauche":"droite", (req==req_right)?neigh_right():neigh_left(),line);fflush(stdout);
#endif

  /* Sauvegarde du dernier envoyé */
  memcpy(comm_sent, last_sent, sizeof (struct comm_t));

  if (where == FROM_RIGHT)
  {
    req = req_right;
    comm = comm_right;
  }
  else if (where == FROM_LEFT)
  {
    req = req_left;
    comm = comm_left;
  }
  else
    abort();

  while (1)
  {
    flag = 0;
    i++;
    usleep(TIME_TO_WAIT);
    MPI_Test(req, &flag, MPI_STATUS_IGNORE);

    if (flag == 1)
    {

      if (comm->flag == TERMINATION)
      { 
        if (where == FROM_LEFT)
        {
          left_terminate = 1;
#ifdef _DEBUG2
          printf("TERMINATION gauche\n");fflush(stdout);
#endif
        }
        else
        {
          right_terminate = 1;
#ifdef _DEBUG_2
          printf("TERMINATION droite\n");fflush(stdout);
#endif
        }
        MPI_Start(req);
        break;
      } 
      else  if (comm->line != line)
      { 
        insert_in_stack(comm, where); 
        MPI_Start(req);
      }
      else
        break;
    }
    else if (i% MAX_ATTEMPT == 0)
    { 
      MPI_Send(comm_sent, sizeof (struct comm_t), MPI_CHAR, (where==TO_RIGHT)?neigh_left():neigh_right(), 1-where, MPI_COMM_WORLD);
#ifdef _DEBUG
      printf("RENVOI %d %d\n", myid, line);
      fflush(stdout);
#endif

    }
    received = request_solve(line, where, comm2);
      
    if (received)
    {
     
    #ifdef _DEBUG
      printf("[%d] On a recu la bonne reponse: ligne %d de %s(%d)\n", myid, line, (where==FROM_LEFT)?"gauche":"droite",
        (where==FROM_RIGHT)?neigh_right():neigh_left());fflush(stdout);
    #endif
      return;
    }
  }

  *comm2 = *comm;
  MPI_Start(req);
#ifdef _DEBUG
  printf("[%d] On a recu la bonne reponse: ligne %d de %s(%d)\n", myid, line, (where==FROM_LEFT)?"gauche":"droite",
    (where==FROM_RIGHT)?neigh_right():neigh_left());fflush(stdout);
#endif
  
}

int
stack_length (void)
{
  int i = 0;
  struct comm_stack *cur = &base_stack;
  while (1)
  {
    cur = cur->next;
    if (cur == &base_stack)
      return i;
    i++;
  }
  return i;
}

int
stack_exists (int id, char where)
{
  struct comm_stack *cur = &base_stack;
  while (1)
  {
    cur = cur->next;
    if (cur == &base_stack)
      return 0;
    if (cur->comm->temoin == id && cur->where == where)
      return 1;
   
  }
  return 0;
}
    

void 
insert_in_stack (struct comm_t *comm, char where)
{
  struct comm_t *comm2 = malloc(sizeof (struct comm_t));
  struct comm_stack *new_s = malloc(sizeof (struct comm_stack));

  if (comm2 == NULL || comm2 == NULL)
  {
    printf("erreur insert_in_stack!\n");
    abort();
  }

  if (stack_exists(comm->temoin, (char)where))
    return;

  *comm2 = *comm;
  new_s->prev = base_stack.prev;
  base_stack.prev = new_s;
  new_s->next = &base_stack;
  new_s->prev->next = new_s;
  
  new_s->where = where;
  new_s->comm = comm2;

#ifdef _DEBUG
  printf("{%d} Insertion... (de la part de %s(%d), ligne %d) \n", myid, (where == FROM_RIGHT)?"droite":"gauche", (where==FROM_RIGHT)?neigh_right():neigh_left(),comm->line); fflush (stdout);
#endif
}

void
delete_from_stack (struct comm_stack *ptr)
{
  struct comm_stack *cur = &base_stack;
    
  while (1)
  {
    cur = cur->next;
    if (cur == &base_stack)
      return;
    if (cur == ptr)
    {       
      cur->prev->next = cur->next;
      cur->next->prev = cur->prev;
        
     free(cur->comm);
      free(cur);
    }
  }
}


void 
handle_request (struct comm_t *comm, int id, char where)
{
  struct comm_t *comm_h;
  fish_t fish;

#ifdef _DEBUG
  printf("[%d, "BLUE"%d\033[00m] REQUETE: Vient de %s(%d), ligne %d. Temoin: %d\n", myid, current_line, (where == FROM_RIGHT)?"droite":"gauche", (where==FROM_RIGHT)?neigh_right():neigh_left(),comm->line, comm->temoin);
  fflush(stdout);
#endif

  if (stack_exists(comm->temoin, where))
    return;

  /* Si on est le dernier proc, et que ça vient du premier */
  if (myid == numprocs - 1 && where == FROM_RIGHT)
  {
    comm_h = malloc(sizeof (struct comm_t));
    comm_h->flag = NOT_THROUGH;
    comm_h->line = comm->line;
    memset(&comm_h->fish, 0, sizeof (struct fish_struct_t));
    comm_h->fish.type = (char)(column_right[comm->line]);
    comm_h->temoin = ++id_packet;
    // Je lui envoie les infos!
    MPI_Send(comm_h, sizeof (struct comm_t), MPI_CHAR, 0, TO_RIGHT, MPI_COMM_WORLD);
    free(comm_h);

    if (comm->flag == GO_THROUGH)
    {
#ifdef _DEBUG
    printf("[%d] he veut GO_THROUGH\n", myid);
#endif
      /* est-ce que le poisson peut venir? */
      if (local_fish_at(bande_width-1, comm->line) == NULL  || local_fish_at(bande_width-1, comm->line)->type == NOTHING)
      {
        if (comm->fish.type == SHARK)
          comm->fish.age_before_starvation--;
        local_replace_at(bande_width-1, comm->line, &comm->fish);
      }
      else if (local_fish_at(bande_width-1, comm->line)->type == SARDINE && comm->fish.type == SHARK)
      {
        comm->fish.age_before_starvation = T_FAMINE;
        local_replace_at(bande_width-1, comm->line, &comm->fish);
      }
    }
    
  
#ifdef _DEBUG
    printf("[%d, "BLUE"%d\033[00m]] ENVOI: pour %s(%d), ligne %d\n", myid, current_line, "droite", neigh_right(), comm->line);fflush(stdout);
#endif    
  }
  /* On regarde si on peut la traiter */
  else if (comm->line < current_line || (comm->line == current_line && where == FROM_LEFT))
  {
    if (where == FROM_RIGHT)
    {
      comm_send (TO_RIGHT, bande_width-1, comm->line, NOT_THROUGH);

      /* On met à jour */
      if (comm->flag == GO_THROUGH)
      {
        /* est-ce que le poisson peut venir? */
        if (local_fish_at(bande_width-1, comm->line) == NULL || local_fish_at(bande_width-1, comm->line)->type == NOTHING)
        {
          if (comm->fish.type == SHARK)
            comm->fish.age_before_starvation--;
          local_replace_at(bande_width-1, comm->line, &comm->fish);
        }
        else if (local_fish_at(bande_width-1, comm->line)->type == SARDINE && comm->fish.type == SHARK)
        {
          comm->fish.age_before_starvation = T_FAMINE;
          local_kill_at(bande_width-1, comm->line);
          local_replace_at(bande_width-1, comm->line, &comm->fish);
        }
      }
    
#ifdef _DEBUG 
      printf("[%d] -> Reponse envoyee, terrain mis a jour ligne %d\n", myid, comm->line);
#endif
    }

    if (where == FROM_LEFT)
    {
      if (myid == 0)
        comm_send (TO_LEFT, 0, comm->line, NOT_THROUGH);
      else
        comm_send_backup (TO_LEFT, comm->line, NOT_THROUGH);      

      /* On met à jour */
      if (comm->flag == GO_THROUGH)
      {
    
        /* est-ce que le poisson peut venir? */
        if ((myid != 0 && column_left[comm->line] == NOTHING) ||
            (myid == 0 && local_fish_at(0, comm->line)->type == NOTHING))
        {
          fish = &comm->fish;
          if (fish->type == SHARK)
            fish->age_before_starvation--;
          local_replace_at(0, comm->line, fish);
        }
        else if (comm->fish.type == SHARK && 
                 ((myid != 0 && column_left[comm->line] == SARDINE) ||
                 (myid == 0 && local_fish_at(0, comm->line)->type == SARDINE)))
        {
          fish = &comm->fish;
          fish->age_before_starvation = T_FAMINE;
          local_replace_at(0, comm->line, fish);
        }
      }
      
#ifdef _DEBUG 
      printf("[%d] -> Reponse envoyee, terrain mis a jour ligne %d\n", myid, comm->line);
#endif
    }
  }

  /* Sinon on met sur la pile */
  else
  {
    insert_in_stack(comm, where);
    fflush(stdout);
  }
}

void 
clear_stack (void)
{
  base_stack.next = &base_stack;
  base_stack.prev = &base_stack;
}

// Scanne la pile et appelle handle_request
int
request_solve (int stop_line, char where, struct comm_t *comm2)
{
  int flag = 0;
  struct comm_stack *cur = &base_stack;
  fish_t fish;
 

  /* On solve à droite */
  MPI_Test(req_right, &flag, MPI_STATUS_IGNORE);
  if (flag == 1)
  {
    if (comm_right->flag == TERMINATION)
    { 
      right_terminate = 1;
#ifdef _DEBUG
      printf("TERMINATION DROITE\n");fflush(stdout);
#endif
      MPI_Start(req_right);
      return 0;
    } 
    else if (comm_right->line == stop_line && where == FROM_RIGHT)
    {
      //free(status);
#ifdef _DEBUG
      printf("ok_r\n");
#endif
      memcpy(comm2, comm_right, sizeof(struct comm_t));
      MPI_Start(req_right);
      return 1;
    } 
    else 
    {
  
      handle_request(comm_right, neigh_right(), FROM_RIGHT); 
      MPI_Start(req_right);
      return 0;
    }
  }
  flag = 0;

  /* On solve à gauche */
  MPI_Test(req_left, &flag, MPI_STATUS_IGNORE);
  if (flag == 1)
  {
#ifdef _DEBUG
    printf("[%d] test ok!\n", myid);fflush(stdout);
#endif
    if (comm_left->flag == TERMINATION)
    { 
      left_terminate = 1;
#ifdef _DEBUG
      printf("TERMINATION GAUCHE\n");fflush(stdout);
#endif
      MPI_Start(req_left);
      return 0;
    }      
    else if (comm_left->line == stop_line && where == FROM_LEFT)
    {
#ifdef _DEBUG
      printf("ok_l\n");
#endif
      memcpy(comm2, comm_left, sizeof(struct comm_t)); 
      MPI_Start(req_left); 
      return 1;
    } 
    else
    {
      handle_request(comm_left, neigh_left(), FROM_LEFT); 
     // insert_in_stack(comm_left, FROM_LEFT);
      MPI_Start(req_left);
      return 0;
    }
  }

#ifdef _DEBUG 
  printf("[%d] On traite la pile...",myid); fflush(stdout);
#endif
  if (stack_length() >= 1)
  {
    /* On scrute la pile */
    while (1)
    {
      cur = cur->next; 
      if (cur == &base_stack)
        break;

#ifdef _DEBUG 
      printf(" Test: %d %d\n", cur->comm->line, cur->where);fflush(stdout);
#endif
      /* Est-ce une réponse à une question ? */
      if (cur->comm->line == stop_line && cur->where == where)
      {
#ifdef _DEBUG
        printf("deja recu\n");fflush(stdout);
#endif
        memcpy(comm2, cur->comm, sizeof(struct comm_t));
        delete_from_stack(cur);
        return 1;
      } 

      /* On peut le faire? */
      if (cur->where == FROM_RIGHT && myid == numprocs-1)
      {
        struct comm_t comm;

        comm.flag = NOT_THROUGH;
        comm.line = cur->comm->line; 
        comm.fish.type = (char)(column_right[cur->comm->line]);
        comm.temoin = ++id_packet;
        // Je lui envoie les infos!
        MPI_Send(&comm, sizeof (struct comm_t), MPI_CHAR, 0, 1, MPI_COMM_WORLD);
        delete_from_stack(cur);
      }
      else if (cur->comm->line < current_line || (cur->comm->line == current_line && cur->where == FROM_LEFT))
      {
        if (cur->where == FROM_RIGHT)
        {
          comm_send (TO_RIGHT, bande_width-1, cur->comm->line, NOT_THROUGH);

          /* On met à jour */
          if (cur->comm->flag == GO_THROUGH)
          {
            /* est-ce que le poisson peut venir? */
            if (local_fish_at(bande_width-1, cur->comm->line) == NULL || local_fish_at(bande_width-1, cur->comm->line)->type == NOTHING)
            {
              fish = malloc(sizeof (struct fish_struct_t));
              if (fish == NULL)
              {
                printf("aaaarg\n");
                MPI_Abort(MPI_COMM_WORLD,1);
              }
              memcpy(fish, &cur->comm->fish, sizeof(struct fish_struct_t));
              if (fish->type == SHARK)
                fish->age_before_starvation--;
              local_replace_at(bande_width-1, cur->comm->line, fish);
            }
            else if (local_fish_at(bande_width-1, cur->comm->line)->type == SARDINE && cur->comm->fish.type == SHARK)
            {
              fish = malloc(sizeof (struct fish_struct_t));
              memcpy(fish, &cur->comm->fish, sizeof (struct fish_struct_t));
              fish->age_before_starvation = T_FAMINE;
              local_kill_at(bande_width-1, cur->comm->line);
              local_replace_at(bande_width-1, cur->comm->line, fish);
            }
          }
        
    #ifdef _DEBUG 
          printf("[%d] -> Reponse envoyee, terrain mis a jour ligne %d\n", myid, cur->comm->line);
    #endif
        }

        if (cur->where == FROM_LEFT)
        {
          if (myid == 0)
            comm_send (TO_LEFT, 0, cur->comm->line, NOT_THROUGH);
          else
            comm_send_backup (TO_LEFT, cur->comm->line, NOT_THROUGH);      

          /* On met à jour */
          if (cur->comm->flag == GO_THROUGH)
          {
        
            /* est-ce que le poisson peut venir? */
            if ((myid != 0 && column_left[cur->comm->line] == NOTHING) ||
                (myid == 0 && local_fish_at(0, cur->comm->line)->type == NOTHING))
            {
              fish = &cur->comm->fish;
              if (fish->type == SHARK)
                fish->age_before_starvation--;
              local_replace_at(0, cur->comm->line, fish);
            }
            else if (cur->comm->fish.type == SHARK && 
                     ((myid != 0 && column_left[cur->comm->line] == SARDINE) ||
                     (myid == 0 && local_fish_at(0, cur->comm->line)->type == SARDINE)))
            {
              fish = &cur->comm->fish;
              fish->age_before_starvation = T_FAMINE;
              local_replace_at(0, cur->comm->line, fish);
            }
          }
          
    #ifdef _DEBUG 
          printf("[%d] -> Reponse envoyee, terrain mis a jour ligne %d\n", myid, cur->comm->line);
    #endif
        }
        // On la traite...
        delete_from_stack(cur);
      }
      // sinon on ne fait rien
    } 
  }
#ifdef _DEBUG 
    printf("fini\n"); fflush(stdout);
#endif
  return 0;
}

  
