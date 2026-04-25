/*
  matrix class
  with routines
  implementations by A.N.Pankratov (pan@impb.ru)
*/

#include <string>
#include <cmath>
#include <sstream>
#include <vector> 
#include "matrix.h"

matrix::matrix(const vector<vector<double>>& A) {
    m = A.size();
    n = 1;
    l = 1;

    for (auto& row : A)
        if (row.size() > n)
            n = row.size();

    p = new double[m * n]; memo = true;
    assert(p != nullptr);

    *this = 0;

    int i = 0;
    for (auto& row : A) {
        int j = 0;
        for (auto& num : row)
            (*this)(i, j++) = num;
        i++;
    }
}

matrix::matrix(const matrix& A) : matrix(A.m, A.n) {
#pragma omp simd
    for (int i = 0; i < m * n; i++)
        p[i] = A.p[i];
}

matrix& matrix::operator =(matrix&& A) noexcept { //A=B+C
    if (memo) {
        delete[] p;
        p = nullptr;
    }
    if (p == nullptr) {
        m = A.m; n = A.n; l = A.l;
        p = A.p; A.p = nullptr;
        memo = A.memo; A.memo = false;
    }
    else
        *this = A;

    return *this;
};

matrix& matrix::operator =(const matrix& A) {
    assert(m == A.m && n == A.n);
    double* p1 = p;
    double* p2 = A.p;
#pragma omp simd
    for (int i = 0; i < m * n; i++)
        p[i] = A.p[i];
    return *this;
}

matrix& matrix::operator =(const double a) {
    double* p1 = p;
#pragma omp simd
    for (int i = 0; i < m * n * l; i++)
        *p1++ = a;
    return *this;
}

matrix& matrix::operator +=(const matrix& A) {
    assert(m == A.m && n == A.n);
#pragma omp simd
    for (int i = 0; i < m * n; ++i)
        (*this)(i) += A(i);
    return *this;
}

matrix& matrix::operator +=(const double a) {
#pragma omp simd
    for (int i = 0; i < m * n; i++)
        (*this)(i) += a;
    return *this;
}

matrix& matrix::operator -=(const matrix& A) {
    assert(m == A.m && n == A.n);
#pragma omp simd
    for (int i = 0; i < m * n; i++)
        (*this)(i) -= A(i);
    return *this;
}

matrix& matrix::operator -=(const double a) {
#pragma omp simd    
    for (int i = 0; i < m * n; i++)
        (*this)(i) -= a;
    return *this;
}

matrix& matrix::operator *=(const double a) {
#pragma omp simd
    for (int i = 0; i < m * n; i++)
        (*this)(i) *= a;
    return *this;
}

matrix& matrix::operator /=(const double a) {
#pragma omp simd
    for (int i = 0; i < m * n; i++)
        (*this)(i) /= a;
    return *this;
}

matrix& matrix::dot(const matrix& A, const matrix& B) {
    assert(m == A.m && n == B.m && (A.n == B.n || l == B.n - A.n));
    (*this) = 0;
#pragma omp parallel for
    for (int lslide = 0; lslide < l; ++lslide)
        for (int j = 0; j < n; ++j)
            for (int k = 0; k < A.n; ++k) {
                double* p1 = &(*this)(0, j, lslide);
                double* p2 = &A(0, k);
                double  p3 = B(j, k + lslide);
#pragma omp simd
                for (int i = 0; i < m; ++i)
                    *p1++ += *p2++ * p3;
                //(*this)(i, j, lslide) += A(i, k) * B(j, k + lslide);
            }
    return *this;
}

matrix matrix::operator *(const matrix& A) const {
    assert(n == A.m);
    matrix R(m, A.n);
    R = 0;
    for (int j = 0; j < A.n; ++j)
        for (int k = 0; k < n; ++k)
#pragma omp simd
            for (int i = 0; i < m; ++i)
                R(i, j) += (*this)(i, k) * A(k, j);
    return R;
}

/*
//large matrix multiplication 
matrix matrix::operator *(const matrix& A) const {
    assert(n == A.m);
    int d = 64, di, dj, dk;
    double* p, * p1, p2;
    matrix R(m, A.n);
    R = 0;
    for (int j = 0; j < A.n; j += d) {
        dj = ((A.n - j) > d) ? d : (A.n - j);
        for (int k = 0; k < n; k += d) {
            dk = ((n - k) > d) ? d : (n - k);
            for (int i = 0; i < m; i += d) {
                di = ((m - i) > d) ? d : (m - i);
                for (int jj = j; jj < j + dj; ++jj) {
                    for (int kk = k; kk < k + dk; ++kk) {
                        p = &R(i, jj);
                        p1 = &(*this)(i, kk);
                        p2 = A(kk, jj);
#pragma omp simd
                        for (int ii = 0; ii < di; ++ii)
                            *p++ += *p1++ * p2;
                        //                            R(ii, jj) += (*this)(ii, kk) * A(kk, jj);
                    }
                }
            }
        }
    }
    return R;
}
*/

matrix matrix::operator *(const double a) const {
    matrix R(m, n);
    double* p1 = p;
    double* p2 = R.p;
#pragma omp simd
    for (int i = 0; i < m * n; ++i)
        *p2++ = *p1++ * a;
    return R;
}

matrix matrix::operator /(const double a) const {
    matrix R(m, n);
    double* p1 = p;
    double* p2 = R.p;
#pragma omp simd
    for (int i = 0; i < m * n; i++)
        *p2++ = *p1++ / a;
    return R;
}

matrix matrix::operator +(const matrix& A) const { 
    assert(m == A.m && n == A.n);
    matrix R(m, n);
        double* p1 = p;
        double* p2 = A.p;
        double* p3 = R.p;
#pragma omp simd
        for (int i = 0; i < m * n; ++i)
            *p3++ = *p1++ + *p2++;
    return R;
}

//matrix matrix::operator +(const double a)const {
//    matrix R(m, n);
//
//#pragma omp simd
//    for (int i = 0; i < m * n; i++)
//        R(i) = (*this)(i) + a;
//    return R;
//}

matrix matrix::operator -(const matrix& A)const {
    assert(m == A.m && n == A.n);
    matrix R(m, n);
    double* p1 = p;
    double* p2 = A.p;
    double* p3 = R.p;
#pragma omp simd
    for (int i = 0; i < m * n; ++i)
        *p3++ = *p1++ - *p2++;
    return R;
}

//matrix matrix::operator -(const double a)const {
//    matrix R(m, n);
//#pragma omp simd
//    for (int i = 0; i < m * n; i++)
//        R(i) = (*this)(i) - a;
//    return R;
//}

//matrix matrix::operator -(void)const {
//    matrix R(m, n);
//#pragma omp simd
//    for (int i = 0; i < m * n; i++)
//        R(i) = -(*this)(i);
//    return R;
//}


bool matrix::operator ==(const matrix& A)const {
    assert(m == A.m && n == A.n);
    for (int i = 0; i < m * n; i++)
        if ((*this)(i) != A(i))
            return false;
    return true;
}

bool matrix::operator>(const double a) const {
    for (int i = 0; i < m * n; i++)
        if ((*this)(i) <= a)
            return false;
    return true;
}

bool matrix::operator<(const double a) const {
    for (int i = 0; i < m * n; i++)
        if ((*this)(i) <= a)
            return false;
    return true;
}

double vecnorm(const matrix& A) {
    double a, r = 0;
#pragma omp simd
    for (int i = 0; i < A.m * A.n; i++) {
        a = abs(A(i));
        if (r < a) r = a;
    }
    return r;
}

double matnorm(const matrix& A) {
    double a, r = 0;
    for (int i = 0; i < A.m; i++) {
        a = 0;
#pragma omp simd
        for (int j = 0; j < A.n; j++)
            a += abs(A(i, j));
        if (r < a) r = a;
    }
    return r;
}

matrix Co(const matrix& A) {
    matrix R(A.n, A.m);
    for (int i = 0; i < A.m; i++)
        for (int j = 0; j < A.n; j++)
            R(j, i) = A(i, j);
    return R;
}

matrix Lo(const matrix& A) {
    assert(A.m == A.n);
    matrix R(A.m, A.n); R = 0;
    for (int i = 0; i < A.m; i++)
        for (int j = 0; j < i; j++)
            R(i, j) = A(i, j);
    return R;
}

matrix Di(const matrix& A) {
    assert(A.m == A.n);
    matrix R(A.m, A.n); R = 0;
    for (int i = 0; i < A.m; i++)
        R(i, i) = A(i, i);
    return R;
}

matrix Up(const matrix& A) {
    assert(A.m == A.n);
    matrix R(A.m, A.n); R = 0;
    for (int i = 0; i < A.m; i++)
        for (int j = i + 1; j < A.n; j++)
            R(i, j) = A(i, j);
    return R;
}

ostream& operator << (ostream& os, const matrix& A) {
    for (int i = 0; i < A.m; i++) {
        for (int j = 0; j < A.n; j++)
            os << A(i, j) << ' ';
        os << endl;
    };
    os << endl;
    return os;
}

istream& operator >> (istream& is, matrix& A) {

    vector<vector<double>> T;
    string show;
    double num;
    bool flag = true;

    while (flag && getline(is, show)) {
        istringstream stream(show);
        vector<double> row;
        while (stream >> num)
            row.push_back(num);
        if ((flag = (row.size() > 0)))
            T.push_back(row);
    }
    A = T;
    return is;
}

void euler(matrix& X, double& t, double h, RHS F) {
    X += h * (*F)(t + h / 2, X + h / 2 * (*F)(t, X));
    t += h;
}

void rk(matrix& X, double& t, double h, RHS F) {
    matrix S, R;
    S = (*F)(t, X);
    S += 2 * (R = (*F)(t += h / 2, X + h / 2 * S));
    S += 2 * (R = (*F)(t, X + h / 2 * R));
    S += (*F)(t += h / 2, X + h * R);
    X += h / 6 * S;
}

void merson(matrix& X, double& t, double& h, double tend, double d, RHS F) {
    matrix X1, X2;
    matrix F1, F2, F3;
    if ((t + h) >= tend)
        h = tend - t;
    X1 = X + h / 3 * (F1 = (*F)(t, X));
    X1 = X + h / 6 * (F1 + (*F)(t + h / 3, X1));
    X1 = X + h / 8 * (F1 + 3 * (F2 = (*F)(t + h / 3, X1)));
    X1 = X + h * (F1 / 2 - 1.5 * F2 + 2 * (F3 = (*F)(t + h / 2, X1)));
    X2 = X + h / 6 * (F1 + F3 + (*F)(t + h, X1));
    double e = vecnorm(X1 - X2);
    if (e > d)
        h /= 2;
    else {
        X = X1; t += h;
        if (e < d / 50)
            h *= 2;
    }
}

void mersonD(matrix& X, matrix& D, double& t, double& h, double tend, double d, RHSD F) {
    matrix X1(&X(0, 0, 1), X.rownum(), X.colnum()), X2(&X(0, 0, 2), X.rownum(), X.colnum());
    matrix D1(&D(0, 0, 1), D.rownum(), D.colnum()), D2(&D(0, 0, 2), D.rownum(), D.colnum());
    if ((t + h) >= tend)
        h = tend - t;
    (*F)(t, X, D);
    X1 = X + h / 3 * D;
    (*F)(t + h / 3, X1, D1);
    X1 = X + h / 6 * (D + D1);
    (*F)(t + h / 3, X1, D1);
    X1 = X + h / 8 * (D + 3 * D1);
    (*F)(t + h / 2, X1, D2);
    X1 = X + h * (D / 2 - 1.5 * D1 + 2 * D2);
    (*F)(t + h, X1, D1);
    X2 = X + h / 6 * (D + D2 + D1);
    double e = vecnorm(X1 - X2);
    if (e > d)
        h /= 2;
    else {
        X = X1; t += h;
        if (e < d / 50)
            h *= 2;
    }
}

void invP(matrix const& P, matrix& R) {
    assert(P.rownum() == P.colnum() && R.rownum() == R.colnum() && P.rownum() == R.colnum());
    int n = P.rownum();
    for (int k = 0; k < n - 1; k++)
        for (int i = k + 1; i < n; i++) {
            R(i, k) = -P(i, k);
            for (int j = k + 1; j < i; j++)
                R(i, k) -= P(i, j) * R(j, k);
        }
}

double compact(matrix const& A, matrix& P, matrix& Q) {
    assert(A.rownum() == A.colnum() && P.rownum() == P.colnum() && Q.rownum() == Q.colnum() &&
        P.rownum() == Q.colnum() && A.rownum() == P.colnum());
    double det = 1;
    int  i, j, k, n = A.rownum();
    for (i = 0; i < n; i++) {
        for (j = i; j < n; j++) {
            Q(i, j) = A(i, j);
            for (k = 0; k < i; k++)
                Q(i, j) -= P(i, k) * Q(k, j);
        }
        if ((det *= Q(i, i)) == 0)
            return 0;
        for (j = i + 1; j < n; j++) {
            P(j, i) = A(j, i);
            for (k = 0; k < i; k++)
                P(j, i) -= P(j, k) * Q(k, i);
            P(j, i) /= Q(i, i);
        }
    }
    return det;
}

void solveP(matrix const& P, matrix& F) {
    assert(P.rownum() == P.colnum() && F.rownum() == P.colnum());
    int n = P.rownum(), m = F.colnum();
    for (int k = 0; k < m; k++)
        for (int i = 1; i < n; i++)
#pragma omp simd
            for (int j = 0; j < i; j++)
                F(i, k) -= P(i, j) * F(j, k);
}

void solveQ(matrix const& Q, matrix& F) {
    assert(Q.rownum() == Q.colnum() && F.rownum() == Q.colnum());
    int n = Q.rownum(), m = F.colnum();
    for (int k = 0; k < m; k++) {
        F(n - 1, k) /= Q(n - 1, n - 1);
        for (int i = n - 2; i >= 0; i--) {
#pragma omp simd
            for (int j = i + 1; j < n; j++)
                F(i, k) -= Q(i, j) * F(j, k);
            F(i, k) /= Q(i, i);
        }
    }
}

double solve(matrix const& A, matrix& F) {
    int n = F.rownum();
    matrix B(n, n); // A=P*Q   B=P-E+Q
    double det = compact(A, B, B);
    if (det == 0) {
        cerr << A << "det==0 in solve";
        exit(3);
    }
    else {
        solveP(B, F);
        solveQ(B, F);
    }
    return det;
}

void spusk(matrix& X0, FUN F) {
    int i, j, k, n = X0.rownum();
    matrix F0(n - 1, 1), F1(n - 1, 1), A(n - 1, n), At(n, n - 1), FF;
    double h = 0.001;
    for (k = 0; k < 3; k++) {
        FF = F(X0);
        if (FF.colnum() == 1) {
            F0 = (matrix&&)FF;
            for (j = 0; j < n; j++) {
                X0(j, 0) += h;
                F1 = F(X0);
                for (i = 0; i < (n - 1); i++)
                    At(j, i) = A(i, j) = (F1(i, 0) - F0(i, 0)) / h;
                X0(j, 0) -= h;
            }
        }
        else if (FF.colnum() == (n + 1))
            for (i = 0; i < (n - 1); i++) {
                F0(i, 0) = FF(i, 0);
                for (j = 0; j < n; j++)
                    At(j, i) = A(i, j) = FF(i, j + 1);
            }
        else {
            cerr << FF << "wrong FUN in curve";
            exit(3);
        }
        solve(A * At, F0);
        X0 -= At * F0;
    }
}

void curve(matrix& X1, matrix& X2, FUN F, int c) {
    matrix X0;
    X0 = X2 * 2 - X1;
    spusk(X0, F);
    switch (c) {
    case    3: X1 = (X1 + X2) / 2; break;
    case    2: X2 = X0; break;
    case (-1): X1 = X0; break;
    default: X1 = X2; X2 = X0; break;
    }
}