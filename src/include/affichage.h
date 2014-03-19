#ifndef AFFICHAGEH
#define AFFICHAGEH

/* flags pour l'affichage, paramétrable à souhait */
#define DISPLAY_INFO       1
#define IMG_EVERY_X_STEPS  1
#define STOP_ON_IMG        0
#define PRINT_TEXT         1
#define PRINT_SDL          1


void init_affichage(int ocean_size);
void draw_zone(int x,int y,int w,int h,int color);
void draw_fish(int x,int y,int type);
void refresh_affichage(void);
#endif
