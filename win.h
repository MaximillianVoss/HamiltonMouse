/*
  class window
  by A.N.Pankratov (pan@impb.ru)
*/
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

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
    void draw_canvas(void);
    void present(void);
    void overlay_cross(double a, double o, double size, int r, int g, int b);
    void overlay_text(double a, double o, const char* text, int r, int g, int b);
    void scale(double amin, double amax, double omin, double omax);
    void pan_pixels(int dx, int dy);
    void zoom_at(int px, int py, double factor);
    void inv_scale(int px, int py, double& a, double& o); 
    void point(double a, double o);
    void line(double a1, double o1, double a2, double o2);
    void lineto(double a, double o);
    void plot(matrix& a, matrix& o); // plot o(:,i) vs a(:,1) for each i
    void dotplot(bool dm(matrix&, int, int), matrix&);
};
