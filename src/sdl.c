#include "sdl.h"

SDL_Surface* affichage=NULL;
Uint32 colors[NBR_COLORS];

/*----------------------------------------------------------------------------*/
void initSDL(int Width,int Height)
{
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    fprintf(stderr, "Erreur à l'initialisation de la SDL : %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }
  atexit(SDL_Quit);
  
  affichage = SDL_SetVideoMode(Width,Height, 32, SDL_SWSURFACE);
  if (affichage == NULL) {
    fprintf(stderr, "Impossible d'activer le mode graphique : %s\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }

  SDL_WM_SetCaption("L'océan", NULL);
  
  colors[clZone1]=makeColorSDL(0x00,0x00,0x88);
  colors[clZone2]=makeColorSDL(0x0,0x33,0x88);   
  colors[clZone3]=makeColorSDL(0x0,0x66,0x88);  
  colors[clShark]=makeColorSDL(0xFF,0x00,0x00);  
  colors[clSardine]=makeColorSDL(0x00,0xFF,0x00);
}
/*----------------------------------------------------------------------------*/
Uint32 makeColorSDL(int R, int G, int B)
{
  return SDL_MapRGB(affichage->format, R, G, B);
}
/*----------------------------------------------------------------------------*/
Uint32 getColorSDL(int color_id)
{
  return colors[color_id];
}
/*----------------------------------------------------------------------------*/
void setPixel(int x, int y, Uint32 coul)
{
  *((Uint32*)(affichage->pixels) + x + y * affichage->w) = coul;
}
/*----------------------------------------------------------------------------*/
inline void setPixelVerif(int x, int y, Uint32 coul)
{
  if (x >= 0 && x < affichage->w &&
      y >= 0 && y < affichage->h)
    setPixel(x, y, coul);
}
/*----------------------------------------------------------------------------*/
inline void echangerEntiers(int* x, int* y)
{
  int t = *x;
  *x = *y;
  *y = t;
}
/*----------------------------------------------------------------------------*/
void RefreshSDL(void)
{
  SDL_UpdateRect(affichage, 0, 0, 0, 0);  
}
/*----------------------------------------------------------------------------*/
void LigneHorizontaleSDL(int x, int y, int w, Uint32 coul)
{
  SDL_Rect r;

  r.x = x;
  r.y = y;
  r.w = w;
  r.h = 1;

  SDL_FillRect(affichage, &r, coul);
}
/*----------------------------------------------------------------------------*/
void LigneVerticaleSDL(int x, int y, int h, Uint32 coul)
{
  SDL_Rect r;

  r.x = x;
  r.y = y;
  r.w = 1;
  r.h = h;

  SDL_FillRect(affichage, &r, coul);
}
/*----------------------------------------------------------------------------*/
void LigneSDL(int x1, int y1, int x2, int y2, Uint32 coul)
{
  int d, dx, dy, aincr, bincr, xincr, yincr, x, y;
  
  if (abs(x2 - x1) < abs(y2 - y1))
  {
    /* parcours par l'axe vertical */

    if (y1 > y2)
    {
      echangerEntiers(&x1, &x2);
      echangerEntiers(&y1, &y2);
    }

    xincr = x2 > x1 ? 1 : -1;
    dy = y2 - y1;
    dx = abs(x2 - x1);
    d = 2 * dx - dy;
    aincr = 2 * (dx - dy);
    bincr = 2 * dx;
    x = x1;
    y = y1;
    
    setPixelVerif(x, y, coul);

    for (y = y1+1; y <= y2; ++y)
    {
      if (d >= 0)
      {
	      x += xincr;
	      d += aincr;
      }
      else
	      d += bincr;

      setPixelVerif(x, y, coul);
    }
  }
  else 
  {
    /* parcours par l'axe horizontal */
    if (x1 > x2)
    {
      echangerEntiers(&x1, &x2);
      echangerEntiers(&y1, &y2);
    }

    yincr = y2 > y1 ? 1 : -1;
    dx = x2 - x1;
    dy = abs(y2 - y1);
    d = 2 * dy - dx;
    aincr = 2 * (dy - dx);
    bincr = 2 * dy;
    x = x1;
    y = y1;

    setPixelVerif(x, y, coul);

    for (x = x1+1; x <= x2; ++x)
    {
      if (d >= 0)
      {
       	y += yincr;
       	d += aincr;
      }
      else
      	d += bincr;

      setPixelVerif(x, y, coul);
    }
  }    
}
/*----------------------------------------------------------------------------*/
void PolylineSDL(point_t *Points, int nbr, Uint32 coul)
{
  int i;
  for(i=1;i<nbr;i++)
    LigneSDL(Points[i-1].x, Points[i-1].y, Points[i].x, Points[i].y, coul);
  LigneSDL(Points[nbr-1].x, Points[nbr-1].y, Points[0].x, Points[0].y, coul);
}
/*----------------------------------------------------------------------------*/
void FillRectSDL(int x, int y, int w, int h, Uint32 coul)
{
  SDL_Rect r;

  r.x = x;
  r.y = y;
  r.w = w;
  r.h = h;

  SDL_FillRect(affichage, &r, coul);
}
/*----------------------------------------------------------------------------*/
void CercleSDL(int cx, int cy, int rayon, int coul)
{
  int d, y, x;

  d = 3 - (2 * rayon);
  x = 0;
  y = rayon;

  while (y >= x) {
    setPixelVerif(cx + x, cy + y, coul);
    setPixelVerif(cx + y, cy + x, coul);
    setPixelVerif(cx - x, cy + y, coul);
    setPixelVerif(cx - y, cy + x, coul);
    setPixelVerif(cx + x, cy - y, coul);
    setPixelVerif(cx + y, cy - x, coul);
    setPixelVerif(cx - x, cy - y, coul);
    setPixelVerif(cx - y, cy - x, coul);

    if (d < 0)
      d = d + (4 * x) + 6;
    else {
      d = d + 4 * (x - y) + 10;
      y--;
    }

    x++;
  }
}
/*----------------------------------------------------------------------------*/
void DisqueSDL(int cx, int cy, int rayon, int coul)
{
  int d, y, x;

  d = 3 - (2 * rayon);
  x = 0;
  y = rayon;

  while (y >= x) {
    LigneHorizontaleSDL(cx - x, cy - y, 2 * x + 1, coul);
    LigneHorizontaleSDL(cx - x, cy + y, 2 * x + 1, coul);
    LigneHorizontaleSDL(cx - y, cy - x, 2 * y + 1, coul);
    LigneHorizontaleSDL(cx - y, cy + x, 2 * y + 1, coul);

    if (d < 0)
      d = d + (4 * x) + 6;
    else {
      d = d + 4 * (x - y) + 10;
      y--;
    }

    x++;
  }
}
/*----------------------------------------------------------------------------*/
