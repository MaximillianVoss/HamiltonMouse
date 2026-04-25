/*
  class window
  Allegro 5 implementation (Fixed for macOS Double Buffering)
  by A.N.Pankratov (pan@impb.ru)
*/
#include <cassert>
#include "win.h"

void initdraw(void) {
    al_init();
    al_init_primitives_addon();
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
    al_set_target_backbuffer(disp);  // Цель — окно
    al_draw_bitmap(canvas, 0, 0, 0); // Рисуем наш накопленный холст
    al_flip_display();               // Выводим на экран
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
