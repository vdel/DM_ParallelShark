#include "affichage.h"
#include "sdl.h"
#include "fish.h"

int SCALE;

/*----------------------------------------------------------------------------*/
void init_affichage(int size)
{
  SCALE=600/size;
  if(SCALE==0) SCALE=1;
  initSDL(SCALE*size,SCALE*size);
}
/*----------------------------------------------------------------------------*/
void draw_zone(int x,int y,int w,int h,int color)
{;
  if(x+w<=ocean_size)
  {
    FillRectSDL(SCALE*x,SCALE*y,SCALE*w,SCALE*h,color==0?getColorSDL(clZone1):(color==1?getColorSDL(clZone2):getColorSDL(clZone3))); 
  }
  else
  {
    FillRectSDL(SCALE*x,SCALE*y,SCALE*(ocean_size-x),SCALE*h,color==0?getColorSDL(clZone1):(color==1?getColorSDL(clZone2):getColorSDL(clZone3)));
    FillRectSDL(0,SCALE*y,SCALE*(w-ocean_size+x),SCALE*h,color==0?getColorSDL(clZone1):(color==1?getColorSDL(clZone2):getColorSDL(clZone3)));    
  }
}
/*----------------------------------------------------------------------------*/
void draw_fish(int x,int y,int type)
{
  if(type==SHARK)
    FillRectSDL(SCALE*x,SCALE*y,SCALE,SCALE,getColorSDL(clShark));
  else if(type==SARDINE)
    FillRectSDL(SCALE*x,SCALE*y,SCALE,SCALE,getColorSDL(clSardine));        
}
/*----------------------------------------------------------------------------*/
void refresh_affichage()
{
  RefreshSDL();
}
/*----------------------------------------------------------------------------*/
