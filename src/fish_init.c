/* fish_init.c : initialise l'ocean primitif */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <mpi.h>

#include "fish.h"


/*
 * parse_file_oceansize
 *
 * Trouve la taille de l'ocean à partir du fichier
 *
 */
void 
parse_file_oceansize (char *filename)
{
  FILE *input_file;
  
  input_file = fopen(filename,"r");
  
  if (!input_file) 
  {
    fprintf(stdout,"Unable to open file %s\n",filename);
    abort();
  }

  if (fscanf(input_file, "%d\n", &ocean_size) != 1) 
  {
    fprintf(stdout, "Unable to read ocean_size in %s\n", filename);
    abort();
  }
  fclose (input_file);
}


/*
 * parse_file_ocean
 *
 * Initialise l'océan à partir du fichier
 *
 */
void
parse_file_ocean (char *filename, int id_proc, int num_proc, int begin, int size) 
{ 
  FILE *input_file;
  int x, y, age;
  char type[20];
  int line = 1;
  int i;

  input_file = fopen(filename,"r");
  if (!input_file) 
  {
    fprintf(stdout,"Unable to open file %s\n",filename);
    abort();
  }
  fscanf(input_file, "%d\n", &ocean_size);  

  bande_begin_x = begin;
  bande_end_x  = (bande_begin_x + size) % ocean_size;
  bande_width = size;

  bande_end_y   = ocean_size;
  bande_height  = ocean_size;
  
  column_left = malloc(ocean_size*sizeof (char));
  column_right = malloc(ocean_size*sizeof (char));
  fish_t fish;

  if (column_left == NULL)
  {
    printf("erreur allocation\n");
    abort();  
  } 

  ocean = (fish_t *) malloc(sizeof(fish_t)*ocean_size * bande_width);

  for (i = 0; i < ocean_size*bande_width; i++)
  {
    ocean[i] = malloc(sizeof(struct fish_struct_t));  
    ocean[i]->type = NOTHING;
  }

  while (fscanf(input_file, "%d %d %s %d\n", &x, &y, type, &age) == 4) 
  {
    if ((x >= bande_begin_x && x < bande_end_x) || (bande_end_x <= bande_begin_x && (x < bande_end_x || x >= bande_begin_x)))
    {
      fish = ocean[((x-bande_begin_x)*(ocean_size))+(y-bande_begin_y)];
      if (strcmp(type, "shark") == 0) 
      {
        fish->type = SHARK;
        nb_of_sharks++;
      } 
      else if (strcmp(type,"sardine") == 0) 
      {
        fish->type = SARDINE;
        nb_of_sardines++;
      } 
      else 
      {
        fprintf(stdout,"Unknown type of fish \"%s\"\n",type);
        abort();
      }
      fish->age = age;
      fish->age_before_starvation = T_FAMINE;
      line++; 
    }
  }
  if (!feof(input_file)) 
  {
    fprintf(stdout,"Problem while parsing line %d of file %s\n", line, filename);
    abort();
  }
}


/*
 * fill_column_left
 *
 * Remplis la colonne de gauche
 *
 */
void
fill_column_left (void)
{
  int y;

  for (y = 0; y < ocean_size; y++)
    column_left[y] = ocean[y]->type;
}


