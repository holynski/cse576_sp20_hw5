#pragma once

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cmath>

#include <algorithm>
#include <string>
#include <vector>

#include "utils.h"

using namespace std;

using T=double;

struct Matrix {
  int rows = 0, cols = 0;
  double *data = nullptr;

  // constructor
  Matrix() = default;
  Matrix(int rows, int cols = 1) : rows(rows), cols(cols), data(nullptr) {
    if (rows * cols)
      data = (double *) calloc(rows * cols, sizeof(double));
  }

  // destructor
  ~Matrix() { free(data); }

  // copy constructor
  Matrix(const Matrix &a) : data(nullptr) { *this = a; }

  // move constructor
  Matrix(Matrix &&a) : data(nullptr) { *this = move(a); }

  // copy assignment
  Matrix &operator=(const Matrix &a) {
    if (this == &a)return *this;

    if (data) {
      free(data);
      data = nullptr;
    }
    rows = a.rows;
    cols = a.cols;
    data = (double *) calloc(rows * cols, sizeof(double));
    memcpy(data, a.data, sizeof(double) * rows * cols);
    return *this;
  }

  // move assignment
  Matrix &operator=(Matrix &&a) {
    if (this == &a)return *this;

    if (data)free(data);

    rows = a.rows;
    cols = a.cols;
    data = a.data;

    a.data = nullptr;
    return *this;
  }

  // access

  double *operator[](int a) { return data + cols * a; }
  const double *operator[](int a) const { return data + cols * a; }

  double &operator()(int row, int col) { return data[cols * row + col]; }
  const double &operator()(int row, int col) const { return data[cols * row + col]; }

  // vector access
  double &operator()(int n) {
    assert(cols == 1 || rows == 1);
    return data[n];
  }
  const double &operator()(int n) const {
    assert(cols == 1 || rows == 1);
    return data[n];
  }
  
  // iterators
  
  double* begin() { return data; }
  double* end() { return data+rows*cols; }
  
  const double* begin() const { return data; }
  const double* end()   const { return data+rows*cols; }
  
  
  // Create matrix

  static Matrix identity_homography(void);
  static Matrix translation_homography(double dx, double dy);
  static Matrix augment(const Matrix &m);
  static Matrix identity(int rows, int cols);

  // print matrix
  void print(int max_rows = -1, int max_cols = -1) const;
  void print_size(void) const { printf("%d %d\n", rows, cols); }
  // transform matrix

  Matrix inverse(void) const;
  Matrix transpose(void) const;
  Matrix exp(void) const;
  Matrix get_row(int i) const;

};

Matrix operator-(const Matrix &a);
Matrix operator+(const Matrix &a);

Matrix operator-(const Matrix &a, const Matrix &b);
Matrix operator+(const Matrix &a, const Matrix &b);
Matrix elementwise_divide(const Matrix &a, const Matrix &b);
Matrix elementwise_multiply(const Matrix &a, const Matrix &b);

Matrix operator*(const Matrix &a, const Matrix &b); // Actual matrix/matrix matrix/vector product


Matrix operator*(double scale, const Matrix &a);
Matrix operator*(const Matrix &a, double scale);

Matrix operator/(double scale, const Matrix &a);
Matrix operator/(const Matrix &a, double scale);

void print_matrix(const Matrix &m);
Matrix LUP_solve(const Matrix &L, const Matrix &U, const Matrix &p, const Matrix &b);
Matrix matrix_invert(const Matrix &m);
Matrix in_place_LUP(Matrix &m);
Matrix random_matrix(int rows, int cols);
Matrix sle_solve(const Matrix &A, const Matrix &b);
Matrix solve_system(const Matrix &M, const Matrix &b);
void test_matrix(void);

inline void assert_same_size(const Matrix &a, const Matrix &b) {
  assert(a.cols == b.cols);
  assert(a.rows == b.rows);
}

struct Matrix2x2 {
  double a, b, c, d;
  Matrix2x2() : a(0), b(0), c(0), d(0) {}
  Matrix2x2(double a, double b, double c, double d) : a(a), b(b), c(c), d(d) {}

  Matrix2x2 inverse(void) const {
    double det = a * d - b * c;
    return Matrix2x2(d / det, -b / det, -c / det, a / det);
  }
};

struct Vector2 {
  double a, b;
  Vector2() : a(0), b(0) {}
  Vector2(double a, double b) : a(a), b(b) {}
};

inline Matrix2x2 operator*(double s, Matrix2x2 m) { return Matrix2x2(m.a * s, m.b * s, m.c * s, m.d * s); }
inline Matrix2x2 operator*(Matrix2x2 m, double s) { return Matrix2x2(m.a * s, m.b * s, m.c * s, m.d * s); }
inline Matrix2x2 operator/(double s, Matrix2x2 m) { return Matrix2x2(m.a / s, m.b / s, m.c / s, m.d / s); }
inline Matrix2x2 operator/(Matrix2x2 m, double s) { return Matrix2x2(m.a / s, m.b / s, m.c / s, m.d / s); }

inline Vector2 operator*(double s, Vector2 m) { return Vector2(m.a * s, m.b * s); }
inline Vector2 operator*(Vector2 m, double s) { return Vector2(m.a * s, m.b * s); }
inline Vector2 operator/(double s, Vector2 m) { return Vector2(m.a / s, m.b / s); }
inline Vector2 operator/(Vector2 m, double s) { return Vector2(m.a / s, m.b / s); }

inline Vector2 operator*(Matrix2x2 m, Vector2 v) { return Vector2(m.a * v.a + m.b * v.b, m.c * v.a + m.d * v.b); }
