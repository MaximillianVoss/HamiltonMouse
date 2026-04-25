/*
  class window
  by A.N.Pankratov (pan@impb.ru)
*/
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include "matrix.h"

extern void initdraw(void);

class win {
    ALLEGRO_DISPLAY* disp;
    ALLEGRO_BITMAP* canvas;
    struct {
        int       size, zero, p;
        double    min, max, d;
    } abs, ord;
public:
    win(int asize, int osize);
    ~win(void);
    ALLEGRO_DISPLAY* display(void) const;
    ALLEGRO_EVENT_SOURCE* event_source(void) const;
    void title(const char* text);
    void position(int x, int y);
    void clear(void);
    void flip(void);
    void scale(double amin, double amax, double omin, double omax);
    void inv_scale(int px, int py, double& a, double& o); 
    void point(double a, double o);
    void lineto(double a, double o);
    void plot(matrix& a, matrix& o); // plot o(:,i) vs a(:,1) for each i
    void dotplot(bool dm(matrix&, int, int), matrix&);
};
