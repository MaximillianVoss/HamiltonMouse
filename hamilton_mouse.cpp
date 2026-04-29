#include <iostream>
#include <cmath>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#ifdef _WIN32
#include <windows.h>
#endif
#include "matrix.h"
#include "win.h"

using namespace std;

struct StationaryPoint {
    double x;
    double y;
};

struct Linearization2D {
    StationaryPoint point;
    double a11;
    double a12;
    double a21;
    double a22;
    double trace;
    double determinant;
    double discriminant;
    double lambda1;
    double lambda2;
    double realPart;
    double imaginaryPart;
    string stabilityType;
};

enum class PhaseMode {
    Nonlinear,
    LinearP1,
    LinearP2
};

using Trajectory = vector<StationaryPoint>;

matrix systemB(double t, const matrix& X) {
    matrix R(2);
    const double x = X(0);
    const double y = X(1);

    // dx/dt = x^2 + y^2 - 2x
    // dy/dt = 3x^2 - x + 3y
    R(0) = x * x + y * y - 2 * x;
    R(1) = 3 * x * x - x + 3 * y;
    return R;
}

vector<StationaryPoint> stationaryPointsSystemB() {
    vector<StationaryPoint> points;

    points.push_back({ 0.0, 0.0 });

    // From g(x,y)=0 we get y = x/3 - x^2.
    // Substitution into f(x,y)=0 gives:
    // x * (x^3 - 2/3*x^2 + 10/9*x - 2) = 0.
    // The cubic has one real root near 1.173.
    const double x = 1.17299672089847;
    const double y = x / 3.0 - x * x;
    points.push_back({ x, y });

    return points;
}

Linearization2D linearizeSystemBAt(const StationaryPoint& point);

string classifyLinearization(double trace, double determinant, double discriminant) {
    const double eps = 1e-9;

    if (determinant < -eps) {
        return "saddle, unstable";
    }
    if (determinant > eps && trace < -eps && discriminant > eps) {
        return "stable node";
    }
    if (determinant > eps && trace > eps && discriminant > eps) {
        return "unstable node";
    }
    if (determinant > eps && trace < -eps && discriminant < -eps) {
        return "stable focus";
    }
    if (determinant > eps && trace > eps && discriminant < -eps) {
        return "unstable focus";
    }
    if (determinant > eps && fabs(trace) <= eps && discriminant < -eps) {
        return "center in linear approximation";
    }
    return "degenerate case, needs additional analysis";
}

string pointLabel(const StationaryPoint& point, int index) {
    const Linearization2D L = linearizeSystemBAt(point);
    ostringstream out;
    out << fixed << setprecision(3);
    out << "P" << index << " " << L.stabilityType;
    out << " (" << point.x << ", " << point.y << ")";
    return out.str();
}

string modeTitle(PhaseMode mode) {
    switch (mode) {
    case PhaseMode::LinearP1:
        return "Mode 2: linearized near P1";
    case PhaseMode::LinearP2:
        return "Mode 3: linearized near P2";
    default:
        return "Mode 1: nonlinear system";
    }
}

void drawPhasePortrait(win& w, bool showLabels, PhaseMode mode) {
    const vector<StationaryPoint> points = stationaryPointsSystemB();

    w.draw_canvas();
    const string title = modeTitle(mode);
    w.overlay_text(-3.9, 3.8, title.c_str(), 0, 0, 160);
    for (int i = 0; i < static_cast<int>(points.size()); ++i) {
        const StationaryPoint& point = points[i];
        w.overlay_cross(point.x, point.y, 6, 220, 0, 0);
        if (showLabels) {
            const string label = pointLabel(point, i + 1);
            w.overlay_text(point.x, point.y, label.c_str(), 180, 0, 0);
        }
    }
    w.present();
}

void redrawPhaseCanvas(win& w, const vector<Trajectory>& trajectories, bool showLabels, PhaseMode mode) {
    w.clear();
    for (const Trajectory& trajectory : trajectories) {
        if (trajectory.size() == 1) {
            w.point(trajectory[0].x, trajectory[0].y);
        }
        for (size_t i = 1; i < trajectory.size(); ++i) {
            w.line(trajectory[i - 1].x, trajectory[i - 1].y, trajectory[i].x, trajectory[i].y);
        }
    }
    drawPhasePortrait(w, showLabels, mode);
}

bool canRedrawNow(double& lastRedrawTime) {
    const double now = al_get_time();
    if (now - lastRedrawTime < 1.0 / 60.0) {
        return false;
    }

    lastRedrawTime = now;
    return true;
}

void coalescePanEvents(ALLEGRO_EVENT_QUEUE* queue, ALLEGRO_DISPLAY* display, int& dx, int& dy) {
    ALLEGRO_EVENT next;
    while (al_peek_next_event(queue, &next)) {
        if (next.type != ALLEGRO_EVENT_MOUSE_AXES ||
            next.mouse.display != display ||
            next.mouse.dz != 0) {
            break;
        }

        al_drop_next_event(queue);
        dx += next.mouse.dx;
        dy += next.mouse.dy;
    }
}

matrix linearizedSystemAt(double t, const matrix& X, const StationaryPoint& point) {
    const Linearization2D L = linearizeSystemBAt(point);
    matrix R(2);
    const double dx = X(0) - point.x;
    const double dy = X(1) - point.y;

    R(0) = L.a11 * dx + L.a12 * dy;
    R(1) = L.a21 * dx + L.a22 * dy;
    return R;
}

matrix linearizedSystemP1(double t, const matrix& X) {
    return linearizedSystemAt(t, X, stationaryPointsSystemB()[0]);
}

matrix linearizedSystemP2(double t, const matrix& X) {
    return linearizedSystemAt(t, X, stationaryPointsSystemB()[1]);
}

RHS rhsForMode(PhaseMode mode) {
    switch (mode) {
    case PhaseMode::LinearP1:
        return linearizedSystemP1;
    case PhaseMode::LinearP2:
        return linearizedSystemP2;
    default:
        return systemB;
    }
}

void resetPlots(win& w, win& w1, win& w2, vector<Trajectory>& trajectories, bool showLabels, PhaseMode mode) {
    trajectories.clear();
    redrawPhaseCanvas(w, trajectories, showLabels, mode);
    w1.clear();
    w1.flip();
    w2.clear();
    w2.flip();
}

Linearization2D linearizeSystemBAt(const StationaryPoint& point) {
    Linearization2D L;

    // J(x,y) = [ 2x - 2   2y ]
    //          [ 6x - 1    3 ]
    L.point = point;
    L.a11 = 2 * point.x - 2;
    L.a12 = 2 * point.y;
    L.a21 = 6 * point.x - 1;
    L.a22 = 3;

    // Trace is the sum of the main diagonal.
    L.trace = L.a11 + L.a22;
    // Determinant for a 2x2 matrix.
    L.determinant = L.a11 * L.a22 - L.a12 * L.a21;
    // Discriminant of lambda^2 - trace*lambda + determinant = 0.
    L.discriminant = L.trace * L.trace - 4 * L.determinant;

    if (L.discriminant >= 0) {
        // Real eigenvalues of the 2x2 matrix.
        L.lambda1 = (L.trace - sqrt(L.discriminant)) / 2;
        L.lambda2 = (L.trace + sqrt(L.discriminant)) / 2;
        L.realPart = 0;
        L.imaginaryPart = 0;
    }
    else {
        // Complex eigenvalues: trace/2 +- i*sqrt(-D)/2.
        L.lambda1 = 0;
        L.lambda2 = 0;
        L.realPart = L.trace / 2;
        L.imaginaryPart = sqrt(-L.discriminant) / 2;
    }

    L.stabilityType = classifyLinearization(L.trace, L.determinant, L.discriminant);
    return L;
}

void printLinearization(const Linearization2D& L, int index) {
    cout << "Stationary point " << index << ": ";
    cout << "x=" << L.point.x << ", y=" << L.point.y << endl;
    cout << "Jacobian:" << endl;
    cout << "[ " << L.a11 << "  " << L.a12 << " ]" << endl;
    cout << "[ " << L.a21 << "   " << L.a22 << " ]" << endl;
    cout << "trace = " << L.trace << endl;
    cout << "determinant = " << L.determinant << endl;
    cout << "discriminant = " << L.discriminant << endl;

    if (L.discriminant >= 0) {
        cout << "lambda1 = " << L.lambda1 << endl;
        cout << "lambda2 = " << L.lambda2 << endl;
    }
    else {
        cout << "lambda1 = " << L.realPart << " - " << L.imaginaryPart << "i" << endl;
        cout << "lambda2 = " << L.realPart << " + " << L.imaginaryPart << "i" << endl;
    }

    cout << "Stability type: " << L.stabilityType << "." << endl << endl;
}

void printSystemBAnalysis() {
    cout << "Task 1.10 b" << endl;
    cout << "dx/dt = x^2 + y^2 - 2x" << endl;
    cout << "dy/dt = 3x^2 - x + 3y" << endl << endl;

    cout << "Full analysis steps:" << endl;
    cout << "1) stationary points" << endl;
    cout << "2) linearization near each stationary point" << endl;
    cout << "3) stability type" << endl;
    cout << "4) phase portrait by Runge-Kutta trajectories" << endl << endl;

    const vector<StationaryPoint> points = stationaryPointsSystemB();
    for (int i = 0; i < static_cast<int>(points.size()); ++i) {
        printLinearization(linearizeSystemBAt(points[i]), i + 1);
    }

    cout << "Windows:" << endl;
    cout << "1) large window: phase trajectory (x,y)" << endl;
    cout << "2) small window: x(t)" << endl;
    cout << "3) small window: y(t)" << endl << endl;
    cout << "Click in the large window to choose the initial point." << endl;
    cout << "Press Esc or close any window to exit." << endl;
    cout << "Press H to hide/show stationary point labels." << endl;
    cout << "Press 1 for nonlinear system portrait." << endl;
    cout << "Press 2 for linearized portrait near P1." << endl;
    cout << "Press 3 for linearized portrait near P2." << endl;
    cout << "Mouse wheel zooms the phase portrait." << endl;
    cout << "Hold the middle mouse button to pan the phase portrait." << endl;
}

int main(int argc, char** argv) {
#ifdef _WIN32
    HANDLE appMutex = CreateMutexA(nullptr, TRUE, "HamiltonMouseSingleInstance");
    if (appMutex && GetLastError() == ERROR_ALREADY_EXISTS) {
        cout << "HamiltonMouse is already running." << endl;
        CloseHandle(appMutex);
        return 0;
    }
#endif

    printSystemBAnalysis();

    initdraw();
    al_install_mouse();
    al_install_keyboard();

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
    bool showLabels = true;
    PhaseMode mode = PhaseMode::Nonlinear;
    vector<Trajectory> trajectories;
    resetPlots(w, w1, w2, trajectories, showLabels, mode);

    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    al_register_event_source(queue, al_get_mouse_event_source());
    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, w.event_source());
    al_register_event_source(queue, w1.event_source());
    al_register_event_source(queue, w2.event_source());

    matrix Y(2);
    bool running = true;
    bool panning = false;
    double lastPanRedrawTime = 0;
    while (running) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(queue, &ev);

        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            running = false;
        }
        else if (ev.type == ALLEGRO_EVENT_KEY_DOWN && ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
            running = false;
        }
        else if (ev.type == ALLEGRO_EVENT_KEY_DOWN && ev.keyboard.keycode == ALLEGRO_KEY_H) {
            showLabels = !showLabels;
            redrawPhaseCanvas(w, trajectories, showLabels, mode);
            cout << "Stationary point labels: " << (showLabels ? "shown" : "hidden") << endl;
        }
        else if (ev.type == ALLEGRO_EVENT_KEY_DOWN && ev.keyboard.keycode == ALLEGRO_KEY_1) {
            mode = PhaseMode::Nonlinear;
            resetPlots(w, w1, w2, trajectories, showLabels, mode);
            cout << modeTitle(mode) << endl;
        }
        else if (ev.type == ALLEGRO_EVENT_KEY_DOWN && ev.keyboard.keycode == ALLEGRO_KEY_2) {
            mode = PhaseMode::LinearP1;
            resetPlots(w, w1, w2, trajectories, showLabels, mode);
            cout << modeTitle(mode) << endl;
        }
        else if (ev.type == ALLEGRO_EVENT_KEY_DOWN && ev.keyboard.keycode == ALLEGRO_KEY_3) {
            mode = PhaseMode::LinearP2;
            resetPlots(w, w1, w2, trajectories, showLabels, mode);
            cout << modeTitle(mode) << endl;
        }
        else if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && ev.mouse.display == w.display() && ev.mouse.button == 3) {
            panning = true;
            lastPanRedrawTime = 0;
        }
        else if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP && ev.mouse.button == 3) {
            panning = false;
            redrawPhaseCanvas(w, trajectories, showLabels, mode);
        }
        else if (ev.type == ALLEGRO_EVENT_MOUSE_AXES && ev.mouse.display == w.display() && ev.mouse.dz != 0) {
            const double zoomFactor = pow(0.85, ev.mouse.dz);
            w.zoom_at(ev.mouse.x, ev.mouse.y, zoomFactor);
            redrawPhaseCanvas(w, trajectories, showLabels, mode);
        }
        else if (ev.type == ALLEGRO_EVENT_MOUSE_AXES && ev.mouse.display == w.display() && panning) {
            int dx = ev.mouse.dx;
            int dy = ev.mouse.dy;
            coalescePanEvents(queue, w.display(), dx, dy);
            w.pan_pixels(dx, dy);
            if (canRedrawNow(lastPanRedrawTime)) {
                redrawPhaseCanvas(w, trajectories, showLabels, mode);
            }
        }
        else if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && ev.mouse.display == w.display() && ev.mouse.button == 1) {
            double mx, my, t = 0, h = 0.001;
            RHS rhs = rhsForMode(mode);
            w.inv_scale(ev.mouse.x, ev.mouse.y, mx, my);

            Y(0) = mx;
            Y(1) = my;
            Trajectory trajectory;
            trajectory.push_back({ Y(0), Y(1) });

            cout << "Initial point: x=" << Y(0) << ", y=" << Y(1) << endl;

            int stepIndex = 0;
            while (t < 5.0 && fabs(Y(0)) < 4.0 && fabs(Y(1)) < 4.0) {
                rk(Y, t, h, rhs);
                ++stepIndex;
                if (stepIndex % 4 == 0) {
                    trajectory.push_back({ Y(0), Y(1) });
                }
                w1.point(t, Y(0));
                w2.point(t, Y(1));
            }
            trajectory.push_back({ Y(0), Y(1) });

            trajectories.push_back(trajectory);
            cout << "Final point: x=" << Y(0) << ", y=" << Y(1) << ", t=" << t << endl;
            redrawPhaseCanvas(w, trajectories, showLabels, mode);
            w1.flip();
            w2.flip();
        }
    }

    al_destroy_event_queue(queue);
#ifdef _WIN32
    if (appMutex) {
        ReleaseMutex(appMutex);
        CloseHandle(appMutex);
    }
#endif
    return 0;
}
