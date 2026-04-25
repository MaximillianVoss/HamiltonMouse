#include <iostream>
#include <cmath>
#include "matrix.h" 
#include "win.h"   

using namespace std;

struct Linearization2D {
    double a11;
    double a12;
    double a21;
    double a22;
    double trace;
    double determinant;
    double discriminant;
    double lambda1;
    double lambda2;
};

matrix systemB(double t, const matrix& X) {
    matrix R(2);
    const double x = X(0);
    const double y = X(1);
    //dx / dt = x ^ 2 + y ^ 2 - 2x
    //dy / dt = 3x ^ 2 - x + 3y
    R(0) = x * x + y * y - 2 * x;
    R(1) = 3 * x * x - x + 3 * y;
    return R;
}

Linearization2D linearizeSystemBAtZero() {
    Linearization2D L;

    // J(x,y) = [ 2x - 2   2y ]
    //          [ 6x - 1    3 ]
    L.a11 = -2;
    L.a12 = 0;
    L.a21 = -1;
    L.a22 = 3;
    //Trace / след матрицы 
    //—лед Ч это сумма элементов главной диагонали
    L.trace = L.a11 + L.a22;
    //Determinant / определитель
    L.determinant = L.a11 * L.a22 - L.a12 * L.a21;
    //Discriminant / дискриминант
    L.discriminant = L.trace * L.trace - 4 * L.determinant;
    //—обственные значени€ матрицы 2x2
    L.lambda1 = (L.trace - sqrt(L.discriminant)) / 2;
    L.lambda2 = (L.trace + sqrt(L.discriminant)) / 2;
    return L;
}

void printSystemBAnalysis() {
    const Linearization2D L = linearizeSystemBAtZero();

    cout << "Task 1.10 b" << endl;
    cout << "dx/dt = x^2 + y^2 - 2x" << endl;
    cout << "dy/dt = 3x^2 - x + 3y" << endl << endl;

    cout << "The zero point is stationary: f(0,0)=0, g(0,0)=0." << endl;
    cout << "Jacobian at (0,0):" << endl;
    cout << "[ " << L.a11 << "  " << L.a12 << " ]" << endl;
    cout << "[ " << L.a21 << "   " << L.a22 << " ]" << endl << endl;

    cout << "Linearized system:" << endl;
    cout << "dx/dt = -2x" << endl;
    cout << "dy/dt = -x + 3y" << endl << endl;

    cout << "trace = " << L.trace << endl;
    cout << "determinant = " << L.determinant << endl;
    cout << "discriminant = " << L.discriminant << endl;
    cout << "lambda1 = " << L.lambda1 << endl;
    cout << "lambda2 = " << L.lambda2 << endl << endl;

    cout << "Stability type: saddle, unstable." << endl << endl;
    cout << "Windows:" << endl;
    cout << "1) large window: phase trajectory (x,y)" << endl;
    cout << "2) small window: x(t)" << endl;
    cout << "3) small window: y(t)" << endl << endl;
    cout << "Click in the large window to choose the initial point." << endl;
}

int main(int argc, char** argv) {
    printSystemBAnalysis();

    initdraw();
    al_install_mouse();

    const int smallWidth = 250;
    const int smallHeight = 250;
    const int mainWidth = 500;
    const int mainHeight = smallHeight * 2;
    const int screenWidth = 1366;
    const int screenHeight = 768;
    const int totalWidth = smallWidth + mainWidth;
    const int totalHeight = mainHeight;
    const int layoutX = (screenWidth - totalWidth) / 2;
    const int layoutY = (screenHeight - totalHeight) / 2;

    win w1(smallWidth, smallHeight);
    win w2(smallWidth, smallHeight);
    win w(mainWidth, mainHeight);

    w1.title("x(t) for system 1.10 b");
    w2.title("y(t) for system 1.10 b");
    w.title("Phase portrait (x,y), system 1.10 b");

    w1.position(layoutX, layoutY);
    w2.position(layoutX, layoutY + smallHeight);
    w.position(layoutX + smallWidth, layoutY);

    w.scale(-4, 4, -4, 4);
    w1.scale(0, 5, -4, 4);
    w2.scale(0, 5, -4, 4);
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
            double mx, my, t = 0, h = 0.001;
            w.inv_scale(ev.mouse.x, ev.mouse.y, mx, my);
    
            Y(0) = mx; Y(1) = my;

            cout << "Initial point: x=" << Y(0) << ", y=" << Y(1) << endl;

            while (t < 5.0 && abs(Y(0)) < 4.0 && abs(Y(1)) < 4.0) {
                rk(Y, t, h, systemB);
                w.point(Y(0), Y(1));
                w1.point(t, Y(0));
                w2.point(t, Y(1));
            }

            cout << "Final point: x=" << Y(0) << ", y=" << Y(1) << ", t=" << t << endl;
            w.flip();
            w1.flip();
            w2.flip();
        }
    }
    al_destroy_event_queue(queue);
    return 0;
}
