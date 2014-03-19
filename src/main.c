#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <mpi.h>
#include <unistd.h>

#include "fish.h"
#include "communication.h"
#include "affichage.h"
#include "sdl.h"

/*
 * Variables globales
 */


int ocean_size    = 0;
int bande_begin_x = 0;
int bande_end_x   = 0;
int bande_width   = 0;

int bande_begin_y = 0;
int bande_end_y   = 0;
int bande_height  = 0;

int current_line = 0;

int myid = 0;
int numprocs = 0;

fish_t *ocean;

int nb_of_sharks = 0;
int nb_of_sardines = 0;

char *column_left;
char *column_right;

int id_packet = 0;

MPI_Request *req_left;
MPI_Request *req_right;

struct comm_t *comm_left;
struct comm_t *comm_right;
struct comm_t *last_sent;

int left_terminate = 0;
int right_terminate = 0;

int laps = 0;

int 
main (int argc, char **argv)
{ 
  MPI_Init (&argc, &argv);
  int max_step,i;
  int signal;
  
  
  /* initialisation du générateur de nombres aléatoires */
  srand(time(NULL));

  /* Récupération des informations MPI */
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);
  

  if (argc < 3) 
  {
    fprintf(stdout,"Usage: %s <ocean_filename> <nb_of_steps>\n", argv[0]);
    exit(1);
  }
  max_step = atoi(argv[2]);

  parse_file_oceansize(argv[1]);

  if (numprocs < 2)
  {
    printf("\n\nIl faut au moins 2 processeurs\n\n");
    MPI_Abort(MPI_COMM_WORLD, 0);
    exit(-1);
  }

  numprocs--;

  /* Si on a trop de processeurs, on en utilise qu'un peu */
  if (numprocs > ocean_size)
    numprocs = ocean_size-1;
 
  /* Si je ne sers à rien, je m'arrête */
  if (myid > numprocs)
    MPI_Finalize();

  /* Le dernier processeur récupère tout le reste de l'océan */
  if (myid +1 == numprocs)
    parse_file_ocean(argv[1], myid, numprocs, (ocean_size/numprocs)*myid, ocean_size - (ocean_size/numprocs)*(numprocs - 1)); 
  else if (myid + 1 < numprocs)
    parse_file_ocean(argv[1], myid, numprocs, (ocean_size/numprocs)*myid, ocean_size/numprocs);

  /* On se présente */
  if (myid == numprocs)
  {
    printf("Je suis le maitre\n");
    fflush(stdout);
  }
  else if (myid == 0)
  {
    printf("Je suis le n°%d et je contiens %d requins\n", myid, num_fish(SHARK));
    fflush(stdout);
    signal = 0;
    MPI_Send(&signal, 1, MPI_INT, neigh_right(), myid, MPI_COMM_WORLD);
    MPI_Recv(&signal, 1, MPI_INT, neigh_left(), neigh_left(), MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }
  else
  {

    MPI_Recv(&signal, 1, MPI_INT, neigh_left(), neigh_left(), MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    FILE *fd=  fopen("test.txt", "a");
    if (signal == neigh_left())
    { 
      fprintf(stdout, "Je suis le n°%d et je contiens %d requins. G:%d D:%d\n", myid, num_fish(SHARK), neigh_left(), neigh_right());
      fprintf(fd, "Je suis le n°%d et je contiens %d requins. G:%d D:%d\n", myid, num_fish(SHARK), neigh_left(), neigh_right());
      signal = myid;
      fflush(stdout);
    }
    fclose(fd);

    MPI_Send(&signal, 1, MPI_INT, neigh_right(), myid, MPI_COMM_WORLD);
  }

  comm_left = malloc(sizeof (struct comm_t));
  comm_right = malloc(sizeof (struct comm_t));

  req_left = malloc (sizeof (MPI_Request));
  req_right = malloc (sizeof (MPI_Request));
  last_sent = malloc(sizeof(struct comm_t));

  if (myid != numprocs)
  {
      MPI_Recv_init(comm_right, sizeof (struct comm_t), MPI_CHAR, neigh_right(), FROM_RIGHT, MPI_COMM_WORLD, req_right);
      MPI_Recv_init(comm_left,  sizeof (struct comm_t), MPI_CHAR, neigh_left(),  FROM_LEFT, MPI_COMM_WORLD,  req_left);

      MPI_Start(req_right);
      MPI_Start(req_left); 

    for (i = 0; i < max_step; i++) 
    { 
      base_stack.next = &base_stack;
      base_stack.prev = &base_stack;

      signal = 0;
      // On attend le signal
      while (1)
      {
        MPI_Recv(&signal, 1, MPI_INT, numprocs, LETSGO, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if (signal == ONYGO)
          break;
        
          int k;
        if (signal == GIVE_HIM_YOUR_RIGHT_FIVE)
        {   
          if (bande_width < 2)
          {
            signal = IDONTHAVEONE;
            MPI_Send(&signal, 1, MPI_INT, numprocs, LETSGO, MPI_COMM_WORLD);
          }
          else
          {
            signal = OKIHAVEONE;


            struct fish_struct_t *fish_to_drop = malloc(sizeof(struct fish_struct_t) * bande_height);
            for (k=0; k < bande_height; k++)
            {
              fish_to_drop[k] = *ocean[(bande_width-1)*bande_height+k];
            }
            MPI_Send(&signal, 1, MPI_INT, numprocs, LETSGO, MPI_COMM_WORLD);
            MPI_Send(fish_to_drop, sizeof(struct fish_struct_t)*bande_height, MPI_CHAR, neigh_right(), 32, MPI_COMM_WORLD);
            bande_width--;
            bande_end_x--;          
            free(fish_to_drop);
          }
        }
        if (signal == TAKE_HIS_HIGH_FIVE)
        { 
          fish_t *tmp_ocean = malloc(sizeof(fish_t)*((bande_width+1)*bande_height));
          struct fish_struct_t *first_column = malloc(sizeof (struct fish_struct_t)*bande_height);

          if (tmp_ocean == NULL || first_column == NULL)
            exit(-1);

          for (k = 0; k < bande_height*(bande_width+1); k++)
          {
            tmp_ocean[k] = malloc(sizeof(struct fish_struct_t));  
            tmp_ocean[k]->type = NOTHING;
          }
        
  
          MPI_Recv(first_column, sizeof(struct fish_struct_t)*bande_height, MPI_CHAR, neigh_left(), 32, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
          for (k = bande_height; k < ((bande_width+1)*bande_height); k++)
          {
            *tmp_ocean[k] = *ocean[k-ocean_size];
          }
          free(ocean);
          ocean = tmp_ocean;
          for (k = 0; k < bande_height; k++)
            *ocean[k] = first_column[k];
          bande_width++;
          bande_begin_x--;
        }
        if (signal == GIVE_HIM_YOUR_LEFT_FIVE)
        {   
          if (bande_width < 2)
          {
            signal = IDONTHAVEONE;
            MPI_Send(&signal, 1, MPI_INT, numprocs, LETSGO, MPI_COMM_WORLD);
          }
          else
          {
            signal = OKIHAVEONE;

            struct fish_struct_t *fish_to_drop = malloc(sizeof(struct fish_struct_t) * bande_height);
            for (k=0; k < bande_height; k++)
            {
              fish_to_drop[k] = *ocean[k];
            }
            MPI_Send(&signal, 1, MPI_INT, numprocs, LETSGO, MPI_COMM_WORLD);
            MPI_Send(fish_to_drop, sizeof(struct fish_struct_t)*bande_height, MPI_CHAR, neigh_left(), 32, MPI_COMM_WORLD);
            bande_width--;
            bande_begin_x++;          
            free(fish_to_drop);
          }
        }
        if (signal == TAKE_HIS_LEFT_FIVE)
        { 
          fish_t *tmp_ocean = malloc(sizeof(fish_t)*((bande_width+1)*bande_height));
          struct fish_struct_t *first_column = malloc(sizeof (struct fish_struct_t)*bande_height);

          if (tmp_ocean == NULL || first_column == NULL)
            exit(-1);

          for (k = 0; k < ocean_size*(bande_width+1); k++)
          {
            tmp_ocean[k] = malloc(sizeof(struct fish_struct_t));  
            tmp_ocean[k]->type = NOTHING;
          }
          MPI_Recv(first_column, sizeof(struct fish_struct_t)*bande_height, MPI_CHAR, neigh_right(), 32, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
          for (k = 0; k < bande_width*ocean_size; k++)
          {
            *tmp_ocean[k] = *ocean[k];
          }
          free(ocean);
          ocean = tmp_ocean;
          for (k = 0; k < bande_height; k++)
            *ocean[bande_width*bande_height+k] = first_column[k];
          bande_width++;
          bande_end_x++;
        }
        if(signal==GIVE_FISH_LIST)
        {
          int numfish;
          int x,y,i;
          struct fish_tab_t *fish_tab;
          numfish=num_fish(SARDINE)+num_fish(SHARK);  
          fish_tab=malloc(sizeof(struct fish_tab_t)*numfish);
          i=0;    
          for(y = 0; y < ocean_size; y++)
            for(x=0; x<bande_width; x++) 
              if (fish_at(x,y) != NULL && fish_at(x,y)->type != NOTHING)        
              {
                fish_tab[i].x=x;
                fish_tab[i].y=y;
                fish_tab[i].type=fish_at(x,y)->type;
                i++;        
              }
          MPI_Send(&bande_width, 1, MPI_INT, numprocs, LETSGO, MPI_COMM_WORLD);
          MPI_Send(&numfish, 1, MPI_INT, numprocs, LETSGO, MPI_COMM_WORLD);      
          MPI_Send(fish_tab, sizeof(struct fish_tab_t)*numfish, MPI_CHAR, numprocs, LETSGO, MPI_COMM_WORLD);
          free(fish_tab); 
        }
      }


#ifdef _DEBUG 
      print_bande();
      printf("[%d] J'ai recu\n", myid);fflush(stdout);
#endif


      left_terminate = 0;
      right_terminate = 0;
      id_packet = 0;

      simulation_step();
      MPI_Send(&laps, 1, MPI_INT, numprocs, LETSGO, MPI_COMM_WORLD); 
       fflush(stdout);
    }
     
  }
 
  // Serveur maitre
  if (myid == numprocs)
  {
    int j;
    int max = 0, min = 0;
    int *time_proc = malloc(sizeof (int) * (numprocs-1));
    int lock = 0;
    if(DISPLAY_INFO && PRINT_SDL)
      init_affichage(ocean_size);
    for (i = 0; i < max_step; i++)
    {
      signal = ONYGO;
      
      for (j = 0; j < numprocs; j++)
      {
        MPI_Send(&signal, 1, MPI_INT, j, LETSGO, MPI_COMM_WORLD);
#ifdef _DEBUG
        printf("%d Message envoye a %d\n",i, j);fflush(stdout);
#endif
      }

      // On attend la terminaison de chaque processus
      for (j = 0; j < numprocs; j++)
      {
      
        MPI_Recv(&time_proc[j], 1, MPI_INT, j, LETSGO, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
#ifdef _RECUT
        printf("Temps du proc %d: %d\n", j, time_proc[j]);
#endif
        if (max <= time_proc[j] || j == 0)
          max = time_proc[j];
        if (min >= time_proc[j] || j == 0)  
          min  = time_proc[j];
  
#ifdef _DEBUG
        printf("%d a termine\n", j);
#endif
      }
#ifdef _RECUT
      printf("Minimum : %d , Maximum: %d\n", min, max);
#endif
      if (i != max_step - 1)
      {
        if (max <= LOAD_UNLOCK*(min/100))
          lock = 0;
        /* Redimmensionnement */
        if (max >= LOAD_LOCK*(min/100) || lock == 1)
        {
          lock = 1;
  #ifdef _RECUT
          printf("Déséquilibre, on reasjute\n");
  #endif
          if (RECUT_OPTION)
          {
            for (j = 0; j < numprocs-1; j++)
            {
              if (time_proc[j] > time_proc[j+1])
              {
                signal = GIVE_HIM_YOUR_RIGHT_FIVE;
                //Envoie une colonne de plus à j+1 
                MPI_Send(&signal, 1, MPI_INT, j, LETSGO, MPI_COMM_WORLD);
                MPI_Recv(&signal, 1, MPI_INT, j, LETSGO, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      
                if (signal == OKIHAVEONE)
                {
                  signal = TAKE_HIS_HIGH_FIVE;
                  MPI_Send(&signal, 1, MPI_INT, j+1, LETSGO, MPI_COMM_WORLD);
                }
              }
              if (time_proc[j] < time_proc[j+1])
              {
                signal = GIVE_HIM_YOUR_LEFT_FIVE;
                // Envoie une colonne de plus à j 
                MPI_Send(&signal, 1, MPI_INT, j+1, LETSGO, MPI_COMM_WORLD);
                MPI_Recv(&signal, 1, MPI_INT, j+1, LETSGO, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      
                if (signal == OKIHAVEONE)
                {
                  signal = TAKE_HIS_LEFT_FIVE;
                  MPI_Send(&signal, 1, MPI_INT, j, LETSGO, MPI_COMM_WORLD);
                }
              }
            }
          }
        }
        if(DISPLAY_INFO && i%IMG_EVERY_X_STEPS==0)
        {
          int n,x,w;
          int numfish;
          int num_sardines,num_sharks;
          int tot_num_sardines,tot_num_sharks;
          tot_num_sardines=0;
          tot_num_sharks=0;
          SDL_Event event;
          signal=GIVE_FISH_LIST;
          
          struct fish_tab_t *fish_tab;
          for (j = 0; j < numprocs; j++)
            MPI_Send(&signal, 1, MPI_INT, j, LETSGO, MPI_COMM_WORLD);
          x=0;
          if(PRINT_TEXT)
            printf("\n\n\n\n\n\nTour %d\n",i);
          for(j = 0; j < numprocs; j++)
          {
            MPI_Recv(&w, 1, MPI_INT, j, LETSGO, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&numfish, 1, MPI_INT, j, LETSGO, MPI_COMM_WORLD, MPI_STATUS_IGNORE);          
            fish_tab=malloc(sizeof(struct fish_tab_t)*numfish);
            MPI_Recv(fish_tab, sizeof(struct fish_tab_t)*numfish, MPI_CHAR, j, LETSGO, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            if(PRINT_SDL)
              draw_zone(x,0,w,ocean_size,j%((numprocs%2==0)?2:3));

            num_sardines=0;
            num_sharks=0;          
            for(n=0;n<numfish;n++)
            {
              if(PRINT_SDL)
                draw_fish(x+fish_tab[n].x,fish_tab[n].y,fish_tab[n].type);            
              if(fish_tab[n].type==SARDINE) num_sardines++;
              if(fish_tab[n].type==SHARK) num_sharks++;                
            }
            tot_num_sardines+=num_sardines;
            tot_num_sharks+=num_sharks;
            if(PRINT_TEXT)
              printf("Zone %d: debut=%d, largeur=%d, #sardines=%d, #requins=%d\n",j+1,x,w,num_sardines,num_sharks);    
            x+=w;
            free(fish_tab);          
          }
          if(PRINT_TEXT)
          {
            printf("------------------------------------\n");
            printf("total: %d sardines, %d requins\n",tot_num_sardines,tot_num_sharks);
          }
          if(PRINT_SDL)
          {
            refresh_affichage();
            if(STOP_ON_IMG)
            {
              printf("Sélectionnez la fenêtre grahique et appuyez sur une touche\n");
              SDL_WaitEvent(&event);
              while(event.type!=SDL_KEYDOWN)
                SDL_WaitEvent(&event);
            }
          }
        }
      }
    } 
   
    printf("[Maitre] Tout est termine\n");fflush(stdout);
  }
  MPI_Finalize();

  
  return 0;
}
