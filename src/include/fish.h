#ifndef FISH_H
#define FISH_H


#define RECUT_OPTION 1

#define DEBUG
#define DISPLAY
#define RECUT

#define BLUE "\033[0;34m"
/* 
 * les paramètres de la simulation 
 *
 */

/* age de reprodution des requins */
#define BREEDING_AGE_SHARK 50

/* age de reprodution des sardines */
#define BREEDING_AGE_SARDINE 15

/* probabilité de reprodution des requins */
#define PROBA_SHARK 0.1

/* probabilité de reprodution des sardines */
#define PROBA_SARDINE 0.3

/* période de famine des requins */
#define T_FAMINE 10

#define SARDINE 1
#define SHARK 2
#define NOTHING 0



struct fish_struct_t 
{
  char type;
  char age;
  char age_before_starvation;
  char not_to_treat;
};

struct move_comm
{
  int x;
  int y;
  struct fish_struct_t fish;
};



typedef struct fish_struct_t *fish_t;

/* DIRECTIONS */
#define D_NORTH 0 
#define D_SOUTH 1
#define D_EAST  2
#define D_WEST  3

/* VARIABLES GLOBALES */
extern int ocean_size;
extern int bande_begin_x;
extern int bande_begin_y;
extern int bande_end_x;
extern int bande_end_y;
extern int bande_width;
extern int bande_height;

extern int left_terminate;
extern int right_terminate;

extern int myid;
extern int numprocs;
extern int current_line;

extern fish_t *ocean;

extern int nb_of_sharks;
extern int nb_of_sardines;

extern char *column_left;
extern char *column_right;
extern int terminate;
extern int laps;
/* Fonctions */

/* fish_init.c */
void parse_file_oceansize (char *);
void parse_file_ocean (char *, int, int, int, int);
void fill_column_left (void);

/* fish_utils.c */
int num_fish (char);
void print_bande (void);

/* simulation.c */
int case_move_x (int, int, char);
int case_move_y (int, int, char);
fish_t fish_request_to (int x, int y, fish_t);
void fish_replace_at(int, int, fish_t );
void fish_erase_at (int, int);
void local_kill_at (int, int);
void local_erase_at (int, int);
void local_replace_at (int, int, fish_t);
char give_rnd_case (void);
void simulation_step (void);



static inline fish_t
fish_at(int x, int y)
{
  return ocean[x*ocean_size+y];
}


static inline fish_t
local_fish_at(int x, int y)
{
  return ocean[x*ocean_size+y];
}

#endif /* FISH_H */

