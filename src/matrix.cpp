#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cmath>

#include <algorithm>
#include <thread>

#include <random>

using namespace std;

#include "matrix.h"

Matrix operator-(const Matrix &a) {
  Matrix p(a.rows, a.cols);
  for (int i = 0; i < p.rows; i++)
    for (int j = 0; j < p.cols; j++)
      p(i, j) = -a(i, j);
  return p;
}

Matrix operator+(const Matrix &a) { return a; }

Matrix elementwise_divide(const Matrix &a, const Matrix &b) {
  assert(a.cols == b.cols);
  assert(a.rows == b.rows);
  Matrix p(a.rows, a.cols);
  for (int i = 0; i < p.rows; i++)
    for (int j = 0; j < p.cols; j++)
      p(i, j) = a(i, j) / b(i, j);
  return p;
}

Matrix elementwise_multiply(const Matrix &a, const Matrix &b) {
  assert(a.cols == b.cols);
  assert(a.rows == b.rows);
  Matrix p(a.rows, a.cols);
  for (int i = 0; i < p.rows; i++)
    for (int j = 0; j < p.cols; j++)
      p(i, j) = a(i, j) * b(i, j);
  return p;
}

template <int TILE>
void do_tile(Matrix& c, const Matrix &a, const Matrix &b, int row, int col)
  {
  using M=double[TILE][TILE];
  
  M C={{0.}};
  
  int ax=min(a.rows-row,TILE);
  int by=min(b.cols-col,TILE);
  
  for(int mid=0;mid<a.cols;mid+=TILE)
    {
    int m=min(a.cols-mid,TILE);
    
    M A;
    M B;
    
    for(int q1=0;q1<ax;q1++)for(int q2=0;q2<m ;q2++)A[q2][q1]=a(row+q1,mid+q2);
    for(int q1=0;q1<m ;q1++)for(int q2=0;q2<by;q2++)B[q1][q2]=b(mid+q1,col+q2);
    
    for(int q3=0;q3<m;q3++)
      for(int q1=0;q1<ax;q1++)for(int q2=0;q2<by;q2++)
        C[q1][q2]+=A[q3][q1]*B[q3][q2];
    
    }
  
  for(int q1=0;q1<ax;q1++)for(int q2=0;q2<by;q2++)c(row+q1,col+q2)=C[q1][q2];
  
  }

template <int TILE>
void gemm_mt(Matrix& c, const Matrix &a, const Matrix &b)
  {
  double flops=double(a.rows)*double(a.cols)*double(b.cols);
  //TIME(2,"\""+to_string(a.rows) + "   " + to_string(a.cols) +"   " + to_string(b.cols)+"    :  "+to_string(flops/1e6)+"    mflops\"    ");
  for(int q1=0;q1<c.rows;q1+=TILE)
    for(int q2=0;q2<c.cols;q2+=TILE)
      do_tile<TILE>(c,a,b,q1,q2);
  }

void gemm(Matrix& p, const Matrix &a, const Matrix &b)
  {
  for (int i = 0; i < p.rows; i++)
    for (int j = 0; j < p.cols; j++)
      for (int k = 0; k < a.cols; k++)
        p(i, j) += a(i, k) * b(k, j);
  }

Matrix operator*(const Matrix &a, const Matrix &b) {
  double flops=double(a.rows)*double(a.cols)*double(b.cols);
  assert(a.cols == b.rows);
  Matrix p(a.rows, b.cols);
  if(flops>(1<<16))gemm_mt<40>(p,a,b);
  else gemm(p,a,b);
  return p;
}

Matrix operator-(const Matrix &a, const Matrix &b) {
  assert(a.cols == b.cols);
  assert(a.rows == b.rows);
  Matrix p(a.rows, a.cols);
  for (int i = 0; i < p.rows; i++)
    for (int j = 0; j < p.cols; j++)
      p(i, j) = a(i, j) - b(i, j);
  return p;
}

Matrix operator+(const Matrix &a, const Matrix &b) {
  assert(a.cols == b.cols);
  assert(a.rows == b.rows);
  Matrix p(a.rows, a.cols);
  for (int i = 0; i < p.rows; i++)
    for (int j = 0; j < p.cols; j++)
      p(i, j) = a(i, j) + b(i, j);
  return p;
}

Matrix operator*(double scale, const Matrix &a) {
  Matrix b = a;
  for (int q1 = 0; q1 < a.rows; q1++)for (int q2 = 0; q2 < a.cols; q2++)b(q1, q2) *= scale;
  return b;
}

Matrix operator*(const Matrix &a, double scale) { return scale * a; }

Matrix operator/(double scale, const Matrix &a) {
  Matrix b = a;
  for (int q1 = 0; q1 < a.rows; q1++)for (int q2 = 0; q2 < a.cols; q2++)b(q1, q2) = scale / b(q1, q2);
  return b;
}

Matrix operator/(const Matrix &a, double scale) {
  Matrix b = a;
  for (int q1 = 0; q1 < a.rows; q1++)for (int q2 = 0; q2 < a.cols; q2++)b(q1, q2) /= scale;
  return b;
}

Matrix Matrix::identity_homography(void) {
  Matrix H(3, 3);
  H(0, 0) = 1;
  H(1, 1) = 1;
  H(2, 2) = 1;
  return H;
}

Matrix Matrix::translation_homography(double dx, double dy) {
  Matrix H = Matrix::identity_homography();
  H(0, 2) = dx;
  H(1, 2) = dy;
  return H;
}

Matrix Matrix::augment(const Matrix &m) {
  Matrix c(m.rows, m.cols * 2);
  for (int i = 0; i < m.rows; i++)
    for (int j = 0; j < m.cols; j++)
      c(i, j) = m(i, j);

  for (int j = 0; j < m.rows; j++)
    c(j, j + m.cols) = 1;

  return c;
}

Matrix Matrix::identity(int rows, int cols) {
  Matrix m(rows, cols);
  for (int i = 0; i < rows && i < cols; i++)
    m(i, i) = 1;
  return m;
}

void Matrix::print(int max_rows, int max_cols) const
{
  const Matrix& m=*this;
  max_rows = (max_rows <= 0 ? m.rows : std::min(m.rows, max_rows));
  max_cols = (max_cols <= 0 ? m.cols : std::min(m.cols, max_cols));

  printf(" __");
  for(int j = 0; j < 16*max_cols-1;j++) printf("_");
  printf("__ \n");

  for(int i = 0; i < max_rows;i++)
  {
    printf("|  ");
    for(int j = 0; j < max_cols;j++)printf("%15.7f ", m(i,j));
    printf(" |\n");
  }

  printf("|__");
  for(int j = 0; j < 16*max_cols-1;j++) printf("_");
  printf("__|\n");
}

void print_matrix(const Matrix &m) { m.print(); }

Matrix LUP_solve(const Matrix &L, const Matrix &U, const Matrix &p, const Matrix &b) {
  Matrix c(L.rows);
  for (int i = 0; i < L.rows; i++) {
    c(i) = b(p(i));
    for (int j = 0; j < i; j++)
      c(i) -= L(i, j) * c(j);
  }

  for (int i = U.rows - 1; i >= 0; --i) {
    for (int j = i + 1; j < U.cols; j++)
      c(i) -= U(i, j) * c(j);
    c(i) /= U(i, i);
  }
  return c;
}


Matrix Matrix::exp(void) const {
  //TIME(2);
  const Matrix &m = *this;
  Matrix t(cols, rows);
  for (int i = 0; i < t.rows; i++) {
    for (int j = 0; j < t.cols; j++) {
      t(i, j) = std::exp(m(i, j));
    }
  }
  return t;
}


Matrix Matrix::get_row(int i) const {
  Matrix out(1, cols);
  for (int j = 0; j < cols; j++) {
    out(j) = (*this)(i, j);
  }
  return out;
}


Matrix Matrix::transpose(void) const {
  //TIME(2);
  const Matrix &m = *this;
  Matrix t(cols, rows);
  for (int i = 0; i < t.rows; i++)
    for (int j = 0; j < t.cols; j++)
      t(i, j) = m(j, i);
  return t;
}

Matrix Matrix::inverse(void) const {
  const Matrix &m = *this;
  //print_matrix(m);
  assert(m.rows == m.cols && "Matrix not square\n");
  Matrix c = Matrix::augment(m);
  //print_matrix(c);

  for (int k = 0; k < c.rows; k++) {
    int index = -1;

    for (int i = k; i < c.rows; i++)
      if (c(i, k) != 0)
        if (index == -1 || fabs(c(i, k)) > fabs(c(index, k)))
          index = i;

    //assert(index != -1 && "Can't invert. Matrix is singular\n");
    if (index == -1) {
      printf("Can't invert. Matrix is singular\n");
      return Matrix::identity(m.rows, m.cols);
    }

    for (int i = 0; i < c.cols; i++)swap(c(k, i), c(index, i));

    for (int j = k + 1; j < c.cols; j++)
      c(k, j) /= c(k, k);

    c(k, k) = 1;

    for (int i = k + 1; i < c.rows; i++) {
      double s = -c(i, k);
      c(i, k) = 0;
      for (int j = k + 1; j < c.cols; j++)
        c(i, j) += s * c(k, j);
    }
  }

  for (int k = c.rows - 1; k > 0; k--)
    for (int i = 0; i < k; i++) {
      double s = -c(i, k);
      c(i, k) = 0;
      for (int j = k + 1; j < c.cols; j++)c(i, j) += s * c(k, j);
    }

  //print_matrix(c);
  Matrix inv(m.rows, m.cols);
  for (int i = 0; i < m.rows; i++)
    for (int j = 0; j < m.cols; j++)
      inv(i, j) = c(i, j + m.cols);
  //print_matrix(inv);
  return inv;
}

// return pivot
Matrix in_place_LUP(Matrix &m) {
  assert(m.rows == m.cols && "Matrix not square\n");

  Matrix pivot(m.rows);

  for (int k = 0; k < m.rows; k++) pivot(k) = k;

  for (int k = 0; k < m.rows; k++) {
    int index = -1;

    for (int i = k; i < m.rows; i++)
      if (m(i, k) != 0)
        if (index == -1 || fabs(m(i, k)) > fabs(m(index, k)))
          index = i;

    assert(index != -1 && "Can't invert. Matrix is singular\n");

    swap(pivot(k), pivot(index));

    for (int i = 0; i < m.cols; i++)swap(m(k, i), m(index, i));

    for (int i = k + 1; i < m.rows; i++) {
      m(i, k) = m(i, k) / m(k, k);
      for (int j = k + 1; j < m.cols; j++)
        m(i, j) -= m(i, k) * m(k, j);
    }
  }
  return pivot;
}

Matrix random_matrix(int rows, int cols) {
  static std::mt19937 mt;
  Matrix m(rows, cols);
  for (int i = 0; i < rows; i++)
    for (int j = 0; j < cols; j++)
      m(i, j) = (int(mt() % 1001u) - 500) / 500.0;
  return m;
}

Matrix sle_solve(const Matrix &A_, const Matrix &b) {
  Matrix A = A_;
  Matrix p = in_place_LUP(A);
  return LUP_solve(A, A, p, b);
}

Matrix solve_system(const Matrix &M, const Matrix &b) {
  Matrix Mt = M.transpose();
  Matrix MtM = Mt * M;
  Matrix MtMinv = MtM.inverse();

  Matrix Mdag = MtMinv * Mt;
  Matrix a = Mdag * b;
  return a;
}

void test_matrix() {
  
  for (int i = 0; i < 100; i++)
    {
    int a=myrand() % 400 + 3;
    int b=myrand() % 400 + 3;
    int c=myrand() % 400 + 3;
    
    
    a=b=c=1024;
    
    Matrix A=random_matrix(a,b);
    Matrix B=random_matrix(b,c);
    Matrix C1(a,c);
    Matrix C2(a,c);
    
    {TIME(1); gemm_mt<40>(C1,A,B);}
    {TIME(1); gemm_mt<32>(C2,A,B);}
    
    //A.print();
    //B.print();
    //C1.print();
    //C2.print();
    //(C1-C2).print();
    
    auto d=C1-C2;
    
    double dd=0;
    for(auto&e1:d)dd+=e1*e1;
    
    if(dd)printf("%18.12g\n",dd);
    
    //exit(0);
    }
  
  
  for (int i = 0; i < 100; i++) {
    int s = myrand() % 40 + 3;
    Matrix m = random_matrix(s, s);
    Matrix inv = m.inverse();
    Matrix res = m * inv;

    //print_matrix(res);

    for (int q1 = 0; q1 < s; q1++)
      for (int q2 = 0; q2 < s; q2++)
        if (fabs(res(q1, q2) - (q1 == q2)) > 1e-6)
          printf("ERROR\n");

    Matrix b = random_matrix(s, 1);

    Matrix c = solve_system(m, b);

    (m * c - b).print();

  }
}
