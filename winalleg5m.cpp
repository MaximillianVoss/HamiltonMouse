/*
  class window
  Allegro 5 implementation (Fixed for macOS Double Buffering)
  by A.N.Pankratov (pan@impb.ru)
*/
#include <cassert>
#include "win.h"

static ALLEGRO_FONT* defaultFont = nullptr;

void initdraw(void) {
    al_init();
    al_init_primitives_addon();
    al_init_font_addon();
    defaultFont = al_create_builtin_font();
}

// Конструктор: создаем окно и холст
win::win(int asize, int osize) {
    disp = al_create_display(asize, osize);
    canvas = al_create_bitmap(asize, osize); // Наш "вечный" холст
    abs.size = asize;
    ord.size = osize;

    // Сразу красим холст в белый цвет
    if (canvas) {
        al_set_target_bitmap(canvas);
        al_clear_to_color(al_map_rgb(255, 255, 255));
    }
}

// Деструктор: чистим память
win::~win(void) {
    if (canvas) al_destroy_bitmap(canvas);
    if (disp) al_destroy_display(disp);
}

ALLEGRO_DISPLAY* win::display(void) const {
    return disp;
}

ALLEGRO_EVENT_SOURCE* win::event_source(void) const {
    return al_get_display_event_source(disp);
}

void win::title(const char* text) {
    al_set_window_title(disp, text);
}

void win::position(int x, int y) {
    al_set_window_position(disp, x, y);
}

void win::scale(double amin, double amax, double omin, double omax) {
    abs.min = amin; ord.min = omin;
    abs.max = amax; ord.max = omax;
    abs.d = (amax - amin) / abs.size;
    ord.d = (omax - omin) / ord.size;
    abs.zero = -amin / abs.d;
    ord.zero = ord.size + omin / ord.d;
}

// Очистка теперь красит именно холст
void win::clear(void) {
    al_set_target_bitmap(canvas);
    al_clear_to_color(al_map_rgb(255, 255, 255));
}

// Главная магия: переносим холст в окно и показываем
void win::flip(void) {
    draw_canvas();
    present();
}

void win::draw_canvas(void) {
    al_set_target_backbuffer(disp);
    al_draw_bitmap(canvas, 0, 0, 0);
}

void win::present(void) {
    al_flip_display();
}

void win::overlay_cross(double a, double o, double size, int r, int g, int b) {
    al_set_target_backbuffer(disp);
    const int x = a / abs.d + abs.zero;
    const int y = ord.zero - o / ord.d;
    const int d = static_cast<int>(size);
    const ALLEGRO_COLOR color = al_map_rgb(r, g, b);
    al_draw_line(x - d, y, x + d, y, color, 2);
    al_draw_line(x, y - d, x, y + d, color, 2);
}

void win::overlay_text(double a, double o, const char* text, int r, int g, int b) {
    if (!defaultFont) return;

    al_set_target_backbuffer(disp);
    const int x = a / abs.d + abs.zero;
    const int y = ord.zero - o / ord.d;
    al_draw_text(defaultFont, al_map_rgb(r, g, b), x + 8, y - 8, 0, text);
}

// Рисование точки на холст
void win::point(double a, double o) {
    al_set_target_bitmap(canvas); // Рисуем в память холста
    int absp = a / abs.d + abs.zero;
    int ordp = ord.zero - o / ord.d;
    al_put_pixel(absp, ordp, al_map_rgb(0, 0, 0));
}

void win::lineto(double a, double o) {
    al_set_target_bitmap(canvas);
    int absp = a / abs.d + abs.zero;
    int ordp = ord.zero - o / ord.d;
    if (abs.p != absp || ord.p != ordp) {
        al_draw_line(abs.p, ord.p, absp, ordp, al_map_rgb(0, 0, 0), 0);
        abs.p = absp;
        ord.p = ordp;
    }
}

void win::plot(matrix& a, matrix& o) {
    assert((a.colnum() == o.colnum()) && (a.rownum() == 1) && (a.colnum() > 0));
    al_set_target_bitmap(canvas);
    for (int i = 0; i < o.rownum(); i++) {
        abs.p = a(0, 0) / abs.d + abs.zero;
        ord.p = ord.zero - o(i, 0) / ord.d;
        for (int j = 1; j < a.colnum(); j++) {
            int absp = a(0, j) / abs.d + abs.zero;
            int ordp = ord.zero - o(i, j) / ord.d;
            al_draw_line(abs.p, ord.p, absp, ordp, al_map_rgb(0, 0, 0), 0);
            abs.p = absp;
            ord.p = ordp;
        }
    }
}

void win::dotplot(bool dm(matrix&, int, int), matrix& A) {
    al_set_target_bitmap(canvas);
    for (int i = 0; i < abs.size; i++)
        for (int j = 0; j < ord.size; j++) {
            if (dm(A, i, j))
                al_put_pixel(i, j, al_map_rgb(0, 0, 0));
        }
}
// Функция перевода пикселей экрана в математические координаты
void win::inv_scale(int px, int py, double& a, double& o) {
    a = (px - abs.zero) * abs.d;
    o = (ord.zero - py) * ord.d;
}
