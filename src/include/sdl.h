#ifndef SDLH
#define SDLH
#include <SDL/SDL.h>

typedef struct point_t
{
  int x;
  int y;
} point_t;

/* Couleurs */
enum {clZone1, clZone2, clZone3, clShark, clSardine, NBR_COLORS};

void initSDL(int Width,int Height);

inline Uint32 makeColorSDL(int R, int G, int B);
Uint32 getColorSDL(int color_id);
inline void setPixel(int x, int y, Uint32 coul);
void LigneSDL(int x1, int y1, int x2, int y2, Uint32 coul);
void PolylineSDL(point_t *Points, int nbr, Uint32 coul);
void RefreshSDL(void);
void LigneHorizontaleSDL(int x, int y, int w, Uint32 coul);
void LigneVerticaleSDL(int x, int y, int w, Uint32 coul);
void FillRectSDL(int x, int y, int w, int h, Uint32 coul);
void CercleSDL(int cx, int cy, int rayon, int coul);
void DisqueSDL(int cx, int cy, int rayon, int coul);
#endif
