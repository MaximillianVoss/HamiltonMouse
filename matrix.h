/*
  matrix class
  with routines
  declarations by A.N.Pankratov (pan@impb.ru)
*/
#pragma once
#include <iostream>
#include <cassert>
#include <vector>

using namespace std;

class matrix {
private:
    double* p;
    int     m, n, l;
    bool    memo;
public:
    matrix() : p(nullptr), m(0), n(0), l(0), memo(false) {};
    matrix(int m, int n = 1, int l = 1) : p(new double[m * n * l]), m(m), n(n), l(l), memo(true) { assert(p != nullptr); };
    matrix(double* p, int m, int n, int l = 1) : p(p), m(m), n(n), l(l), memo(false) {};
    matrix(const vector<vector<double>>&);
    matrix(matrix&& A) noexcept : p(A.p), m(A.m), n(A.n), l(A.l), memo(A.memo) { A.memo = false; };
    matrix(const matrix&);
    ~matrix() { if (memo) delete[] p; }
    int      rownum() const { return m; }
    int      colnum() const { return n; }
    int      matnum() const { return l; }
    double& operator () (int i)               const { assert(i >= 0 && i < m * n * l); return *(p + i); }                                            // flat array
    double& operator () (int i, int j)        const { assert(i >= 0 && i < m && j >= 0 && j < n); return *(p + i + j * m); }                            // column-wise matrix
    double& operator () (int i, int j, int k) const { assert(i >= 0 && i < m && j >= 0 && j < n && k >= 0 && k < l); return *(p + (k * n + j) * m + i); }  // array of column-wise matrices 
    //double& operator () (int i, int j = 0, int k = 0) const { assert(i >= 0 && ((i < m && j >= 0 && j < n) || (i < m * n && j == 0)) && k >= 0 && k < l); return *(p + (k * n + j) * m + i); } // 
    matrix& operator =   (matrix&&) noexcept;  // A=B+C; rvalue  
    matrix& operator =   (const matrix&);      // A=B; lvalue
    matrix& operator =   (const double);
    matrix& operator +=  (const matrix&);
    matrix& operator +=  (const double);
    matrix& operator -=  (const matrix&);
    matrix& operator -=  (const double);
    matrix& operator *=  (const double);
    matrix& operator *=  (const matrix& A) { return *this = *this * A; }
    matrix& operator /=  (const double);
    matrix& dot(const matrix&, const matrix&); //matrix multiplication by transposed matrix A=B*Co(C), i.e. dot or inner product with tensor extension on sliding window
    matrix   operator *  (const matrix&)   const;
    matrix   operator *  (const double)    const;
    matrix   operator /  (const double)    const;
    matrix   operator +  (const matrix&)   const;
//    matrix   operator +  (const double)    const;
    matrix   operator -  (const matrix&)   const;
//    matrix   operator -  (const double)    const;
//    matrix   operator -  (void)            const;
//another operations
    bool     operator == (const matrix&)   const;
    bool     operator != (const matrix& A) const { return !(*this == A); }
    bool     operator >  (const double)    const;
    bool     operator <  (const double)    const;
    bool     operator >= (const double a)  const { return !(*this < a); }
    bool     operator <= (const double a)  const { return !(*this > a); }
//    friend  matrix   operator +  (const double a, const matrix& A) { return A + a; }
//    friend  matrix   operator -  (const double a, const matrix& A) { return -A + a; }
    friend  matrix   operator *  (const double a, const matrix& A) { return A * a; }
    friend  double   vecnorm(const matrix&);
    friend  double   matnorm(const matrix&);
    friend  matrix   Co(const matrix&);
    friend  matrix   Lo(const matrix&);
    friend  matrix   Di(const matrix&);
    friend  matrix   Up(const matrix&);
    friend  ostream& operator << (ostream&, const matrix&);
    friend  istream& operator >> (istream&, matrix&);
};

typedef matrix(*RHS) (double t, const matrix& X);
typedef matrix(*FUN) (const matrix& X);

typedef void(*RHSD)(double t, const matrix& X, matrix& D);

void euler(matrix& X, double& t, double h, RHS F);
void rk(matrix& X, double& t, double h, RHS F);
void merson(matrix& X, double& t, double& h, double tend, double d, RHS F);
void mersonD(matrix& X, matrix& D, double& t, double& h, double tend, double d, RHSD F);

void invP(const matrix& P, matrix& R);                   //P*R=1
double compact(const matrix& A, matrix& P, matrix& Q);   //A=P*Q   
void solveP(const matrix& P, matrix& F);                 //P*X=F   
void solveQ(const matrix& Q, matrix& F);                 //Q*X=F
double solve(const matrix& A, matrix& F);                //A*X=F => P*Q*X=F

void spusk(matrix& X0, FUN F);                        //curve engine
void curve(matrix& X1, matrix& X2, FUN F, int c = 1); //curve stepper