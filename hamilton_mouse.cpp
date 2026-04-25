#include <iostream>
#include <cmath>
#include "matrix.h" 
#include "win.h"   

double F(double x, double y) {
    return -((x - 1) * (x - 1) + y * y - 1) * ((x + 1) * (x + 1) + y * y - 1);
}

matrix G(double t, const matrix& X) { // Gradient
    matrix R(2);
    const double h_diff = 0.001;
    R(1) = (F(X(0) + h_diff, X(1)) - F(X(0) - h_diff, X(1))) / (2 * h_diff);
    R(0) = (F(X(0), X(1) + h_diff) - F(X(0), X(1) - h_diff)) / (2 * h_diff);
    return R;
}

matrix H(double t, const matrix& X) { // Hamiltonian
    matrix R(2);
    const double h_diff = 0.001;
    R(0) = (F(X(0), X(1) + h_diff) - F(X(0), X(1) - h_diff)) / (2 * h_diff);
    R(1) = -(F(X(0) + h_diff, X(1)) - F(X(0) - h_diff, X(1))) / (2 * h_diff);
    return R;
}

int main(int argc, char** argv) {
    initdraw();
    al_install_mouse();
    win w(500, 500);
    win w1(250, 250);
    win w2(250, 250);
    w.scale(-3, 3, -3, 3);
    w1.scale(0, 1, -3, 3);
    w2.scale(0, 1, -3, 3);
    w.clear();
    w.flip();
    w1.clear();
    w1.flip();
    w2.clear();
    w2.flip();

    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    al_register_event_source(queue, al_get_mouse_event_source());

    matrix Y(2);
    bool running = true;
    while (running) {
        ALLEGRO_EVENT ev;
        
        al_wait_for_event(queue, &ev);

            if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            double mx, my, x = 0, h = 0.001, d = 0.01;
            w.inv_scale(ev.mouse.x, ev.mouse.y, mx, my);
            //cout << mx << ' ' << my << endl;
    
            Y(0) = mx; Y(1) = my;

            cout << F(Y(0), Y(1)) << ' ';

            while (x < 100.0) {
                //merson(Y, x, h, 2.0, d, G);
                rk(Y, x, h, G);
                w.point(Y(0), Y(1));
                w1.point(x, Y(0));
                w2.point(x, Y(1));
                //w.flip();
            }
            cout << F(Y(0), Y(1)) << endl;
            w.flip();
            w1.flip();
            w2.flip();
        }
    }
    al_destroy_event_queue(queue);
    return 0;
}
