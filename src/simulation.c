/* simulation.c */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <mpi.h>
#include <sys/time.h>

#include "fish.h"
#include "communication.h"

char 
give_rnd_case (void)
{
  return (rand()%4);
}     

int
case_move_x (int x, int y, char choice)
{
  switch (choice)
  {
    case D_NORTH:
    case D_SOUTH: 
      return x;
  
    case D_EAST:
      return ((x+1)%ocean_size);
    case D_WEST:
      if (x == 0)
        return (ocean_size - 1);
      else
        return x-1;
  }
  return x;
}

int
case_move_y (int x, int y, char choice)
{
  switch (choice)
  {
    case D_NORTH:
      if (y == 0)
        return (ocean_size - 1);
      else
        return y-1;
      
    case D_SOUTH: 
      return ((y+1)%ocean_size);
  
    case D_EAST:
    case D_WEST:
      return y;
  }
  return y;
}



void
local_kill_at (int x, int y)
{ 
  if (x < bande_width)
  {
    ocean[x*ocean_size+y]->type = NOTHING;
  }
}

void
local_erase_at (int x, int y)
{ 
  if (x < bande_width)
    ocean[x*ocean_size+y]->type = NOTHING;
}

void
local_replace_at (int x, int y, fish_t fish)
{ 
  if (x < bande_width && y < bande_height)
  {
    memcpy(ocean[x*ocean_size+y], fish, sizeof(struct fish_struct_t));
  }
}

void
simulation_step(void)
{
  int x, y;
  fish_t fish_src;
  fish_t fish_dest;
  char choice;
  double rand_num;
  int frontier_through = 0;
  int move_success     = 0;
  int dont_act         = 0;
  int dontreplace      = 0;

  column_right = malloc(bande_height*4096);


  struct comm_t comm;
  struct comm_t comm_s;


  struct timeval deb, fin;
  gettimeofday(&deb, NULL);

  clear_stack ();

  /* Si je suis le dernier processeur, je dois garder en mémoire ma dernière colonne */  
  if (myid == numprocs-1)
  {
    for (y = 0; y < bande_height; y++)
      column_right[y] = ocean[(bande_width-1)*ocean_size + y]->type;
  }
  // Je garde en mémoire ma première colonne
  for (y = 0; y < bande_height; y++)
    column_left[y] = ocean[y]->type;

  
  for(y = 0; y < ocean_size; y++) 
  {
    current_line = y;
#ifdef _DISPLAY
    printf("[%d] "BLUE"Simulation ligne n°%d\033[00m\n", myid, y);fflush(stdout);
    
#endif
    // Traite les demandes des voisins 
    request_solve(bande_height+1, 0, &comm_s); 
    for(x = 0; x < bande_width; x++) 
    {
      frontier_through  = 0;
      move_success      = 0;
      dont_act          = 0;
      dontreplace       = 0;

      fish_src = fish_at(x,y);
      
      if (fish_src == NULL)
        continue;
      if (fish_src->type == NOTHING)
        continue;

      choice = give_rnd_case();
      ((int)fish_src->age)++;

      /* Si on tente de traverser la frontière, on doit récupérer fish_dest */
      if (x == 0 && choice == D_WEST)  
      {
#ifdef _DEBUG
        printf("[%d] \033[31mCas 1\033[00m %d\n", myid, column_right[y]);fflush(stdout);
#endif
        
        comm_send(TO_LEFT, x, y, GO_THROUGH); 
        waiting_loop (FROM_LEFT,y,   &comm);
        if (comm.fish.type == NOTHING)
          fish_dest = NULL;
        else
        {
          fish_dest = &comm.fish;
        }
        frontier_through = 1;        
      }
      else if (x == bande_width - 1 && choice == D_EAST)
      {
#ifdef _DEBUG
        printf("[%d] \033[31mCas 2\033[00m %d\n", myid, local_fish_at(x,y)->type);fflush(stdout);
#endif
        comm_send(TO_RIGHT, x, y, GO_THROUGH);  
        waiting_loop(FROM_RIGHT,y, &comm); 
        if (comm.fish.type == NOTHING)
          fish_dest = NULL;
        else
        {
          fish_dest = &comm.fish;
        }
        frontier_through = 1;    
      }
      /* Un poisson veut traverser à gauche */
      else if (x == 0 && fish_src->type == SARDINE)
      {
#ifdef _DEBUG
        printf("[%d] \033[31mCas 3\033[00m\n", myid);fflush(stdout);
#endif
        comm_send(TO_LEFT, x, y, NOT_THROUGH);
        waiting_loop(FROM_LEFT,y,   &comm);
        if (comm.fish.type == NOTHING)
          fish_dest = NULL;
        else
        {
          fish_dest = &comm.fish;
        }
        if (comm.flag == GO_THROUGH && fish_dest->type == SHARK)
        {
          dont_act = 1;
          local_replace_at(x,y,fish_dest);
        }
      }
      /* Cas où le poisson va vers le haut, et que cette case est vide */
      else if (x == bande_width - 1 
               && choice == D_NORTH 
               && y != 0
               && local_fish_at(x,y-1)->type == NOTHING)
      { 
#ifdef _DEBUG
        printf("[%d] \033[31mCas 4\033[00m\n", myid);fflush(stdout);
#endif
        comm_send(TO_RIGHT, x, y-1, NOT_THROUGH);

        waiting_loop(FROM_RIGHT,y-1,   &comm);  
        if (comm.flag == GO_THROUGH)
        {
          fish_dest = &comm.fish;
          dontreplace = 1;
        }
      } 

      /* Cas où le poisson va vers le bas, et que cette case est vide */
      else if (x == 0
               && choice == D_NORTH 
               && y != 0
               && local_fish_at(0,y-1)->type == NOTHING)
      { 
#ifdef _DEBUG
        printf("[%d] \033[31mCas 5\033[00m\n", myid);fflush(stdout);
#endif
        comm_send(TO_LEFT, x, y-1, NOT_THROUGH);
        waiting_loop(FROM_LEFT,y-1,   &comm);  
        if (comm.flag == GO_THROUGH)
        {
          fish_dest = &comm.fish;
          dontreplace = 1;
        }
      }

      /* Cas où le poisson de la 2e case veut aller à gauche */
      else if (x == 1
               && choice == D_WEST 
               && local_fish_at(0,y)->type == NOTHING)
      {  
#ifdef _DEBUG
        printf("[%d] \033[31mCas 6\033[00m ligne %d\n", myid, y);fflush(stdout);
#endif
        comm_send(TO_LEFT, x, y, NOT_THROUGH);
        waiting_loop(FROM_LEFT,y,  &comm);  
        if (comm.flag == GO_THROUGH)
        {
          fish_dest =&comm.fish;
          dontreplace = 1;
        }
      }

      /* Cas où le poisson va vers le bas, et que cette case est une sardine */
      else if (x == bande_width - 1 
               && choice == D_NORTH 
               && y != 0
               && local_fish_at(x,y-1)->type == SARDINE)
      { 
#ifdef _DEBUG
        printf("[%d] \033[31mCas 7\033[00m ligne %d\n", myid, y);fflush(stdout);
#endif
        comm_send(TO_RIGHT, x, y-1, NOT_THROUGH);
        waiting_loop(FROM_RIGHT,y-1,   &comm);  
        if (comm.flag == GO_THROUGH && comm.fish.type == SHARK)
        {
          fish_dest = &comm.fish;
          local_replace_at(x,y-1, fish_dest);
          dontreplace = 1;
        }
      } 

      /* Cas où le poisson va vers le bas, et que cette case est une sardine */
      else if (x == 0
               && choice == D_NORTH 
               && y != 0
               && local_fish_at(0,y-1)->type == SARDINE)
      { 
#ifdef _DEBUG
        printf("[%d] \033[31mCas 8\033[00m ligne %d\n", myid, y);fflush(stdout);
#endif
        comm_send(TO_LEFT, x, y-1, NOT_THROUGH);
        waiting_loop(FROM_LEFT,y-1,   &comm);  
        if (comm.flag == GO_THROUGH && comm.fish.type == SHARK)
        {
          fish_dest = &comm.fish;
          local_replace_at(x,y-1, fish_dest);
          dontreplace = 1;
        }
      }

      /* Cas où le poisson de la 2e case veut aller à gauche */
      else if (x == 1
               && choice == D_WEST 
               && local_fish_at(0,y)->type == SARDINE)
      { 
#ifdef _DEBUG
        printf("[%d] \033[31mCas 9\033[00m ligne %d\n", myid, y);fflush(stdout);
#endif
        comm_send(TO_LEFT, x, y, NOT_THROUGH);
        waiting_loop(FROM_RIGHT,y,   &comm);  
        if (comm.flag == GO_THROUGH && comm.fish.type == SHARK)
        {
          fish_dest = &comm.fish;
          local_replace_at(0, y, fish_dest);
          dontreplace = 1;
        }
      }

      /* RESOLUTION NORMALE */
      switch (fish_src->type)
      {
        case SHARK:
          fish_src->age_before_starvation--;

          /* famine */
          if (!fish_src->age_before_starvation)
            local_kill_at(x,y);

          /* Si on n'a pas dépassé de frontière, on récupère le poisson dest */
          if (frontier_through == 0 && dontreplace == 0)
            fish_dest = local_fish_at(case_move_x(x, y, choice), 
                                      case_move_y(x, y, choice));

          /* S'il n'y a rien, on se déplace */
          if (fish_dest == NOTHING || fish_dest->type == NOTHING)      
          {
#ifdef _DEBUG
              printf("On se déplace\n");fflush(stdout);
#endif
            if (frontier_through == 0)
            {
              local_replace_at(case_move_x(x, y, choice),
                               case_move_y(x, y, choice), fish_src);
              local_erase_at(x, y);
            }
            else
            {
#ifdef _DEBUG
              printf("On efface \n");fflush(stdout);
#endif
              local_kill_at(x, y);
            }

            move_success = 1;

            /* bébé requin? */
            if (fish_src->age >= BREEDING_AGE_SHARK)
            {
              rand_num = ((double)rand())/(RAND_MAX+1.0);  
              if (rand_num <= PROBA_SHARK)
              {
                local_fish_at(x,y)->type = SHARK;
                local_fish_at(x,y)->age = 0;
                local_fish_at(x,y)->age_before_starvation = SHARK;
              }
            }
           
          }
  
          /* Si on tombe sur une sardine */
          else if (fish_dest->type == SARDINE)
          {
            fish_src->age_before_starvation = T_FAMINE;
            if (frontier_through == 0)
            {
              local_replace_at(case_move_x(x, y, choice), 
                               case_move_y(x, y, choice), fish_src);

              local_erase_at(x, y);
#ifdef _DEBUG
              printf("On a bouffé une sardine\n");fflush(stdout);
#endif
            }
            else
              local_kill_at(x, y);   

            move_success = 1;         

            /* bébé requin? */
            if (fish_src->age >= BREEDING_AGE_SHARK)
            {
              rand_num = ((double)rand())/(RAND_MAX+1.0);  
              if (rand_num <= PROBA_SHARK)
              {
                local_fish_at(x,y)->type = SHARK;
                local_fish_at(x,y)->age = 0;
                local_fish_at(x,y)->age_before_starvation = SHARK;
              }
            }
          }

          /* Si on tombe sur un requin... */
          else if (fish_dest->type == SHARK)
          {            
            /* famine */
            if (!fish_src->age_before_starvation)
              local_kill_at(x,y);
          }
                    
          break;

        case SARDINE:
          /* Si on n'a pas dépassé de frontière, on récupère le poisson dest */
          if (frontier_through == 0)
            fish_dest = local_fish_at(case_move_x(x, y, choice), 
                                      case_move_y(x, y, choice));

          /* S'il n'y a rien, on se déplace */
          if (fish_dest == NOTHING || fish_dest->type == NOTHING)      
          {
            if (frontier_through == 0)
            {
              local_replace_at(case_move_x(x, y, choice),
                               case_move_y(x, y, choice), fish_src);
              local_erase_at(x, y);
            }

            local_kill_at(x, y);

            move_success = 1;

            /* bébé sardine? */

            if (fish_src->age >= BREEDING_AGE_SARDINE)
            {
              rand_num = ((double)rand())/(RAND_MAX+1.0);  
              if (rand_num <= PROBA_SARDINE)
              {
                local_fish_at(x,y)->type = SARDINE;
                local_fish_at(x,y)->age = 0;
              }
            }
          }   
  
      }

      /* Actualisation de la première colonne */
      if (x == 0 && y < ocean_size - 1 && choice == D_SOUTH && move_success)
        column_left[y+1] = column_left[y];

      /* Actualisation de la première colonne */
      if (x == bande_width-1 && y < ocean_size && choice == D_SOUTH && move_success)
      {
        column_right[(y+1)%ocean_size] = column_right[y];
      }
      
    }

  }

#ifdef _DISPLAY
  printf("\n\n%d a termine!!\n\n", myid);
  print_bande();
#endif
  current_line++;
  
  gettimeofday(&fin, NULL);

  laps = ((fin.tv_sec*1000000+fin.tv_usec) - (deb.tv_sec*1000000+deb.tv_usec));

  // Synchronisation des montres
  MPI_Request req1, req2;

  comm.flag = TERMINATION;    

  MPI_Isend(&comm, sizeof (struct comm_t), MPI_CHAR, neigh_right(), FROM_LEFT, MPI_COMM_WORLD, &req1);
  MPI_Isend(&comm, sizeof (struct comm_t), MPI_CHAR, neigh_left(), FROM_RIGHT, MPI_COMM_WORLD, &req2);


  while (1)
  {    
    usleep(TIME_TO_WAIT);
    if (left_terminate && right_terminate)
      break;
    request_solve(bande_height+1, 0, &comm_s); 
  }

}
