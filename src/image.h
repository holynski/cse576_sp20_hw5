#pragma once

#include <cassert>
#include <cstring>
#include <cmath>

#include <algorithm>
#include <string>
#include <vector>
#include <stdexcept>

using namespace std;

#include "utils.h"
#include "matrix.h"

// DO NOT CHANGE THIS FILE

struct Image
  {
  int w=0;
  int h=0;
  int c=0;
  float* data=nullptr;
  
  // constructor
  Image() = default;
  
  Image(int w, int h, int c=1) : w(w), h(h), c(c), data(nullptr) 
    {
    assert(c>=0 && w>=0 && h>=0 && "Invalid image sizes");
    
    if(w*h*c)
      data=(float*)calloc(w*h*c,sizeof(float));
    
    }
  
  // destructor
  ~Image() { free(data); }
  
  // copy constructor
  Image(const Image& from) : data(nullptr) { *this=from; }
  
  // move constructor
  Image(Image&& from)  : data(nullptr) { *this=move(from); }
  
  // copy assignment
  Image& operator=(const Image& from)
    {
    if(this==&from)return *this;
    
    if(data){free(data);data=nullptr;}
    w=h=c=0;
    // allocating data for the new image
    data=(float*)calloc(from.w*from.h*from.c,sizeof(float));
    
    // TODO: populate the remaining fields in 'to' and copy the data
    // You might want to check how 'memcpy' function works
    
    w=from.w;
    h=from.h;
    c=from.c;
    memcpy(data,from.data,sizeof(float)*w*h*c);
    return *this;
    }
  
  // move assignment
  Image& operator=(Image&& from)
    {
    if(this==&from)return *this;
    
    if(data)free(data);
    
    w=from.w;
    h=from.h;
    c=from.c;
    data=from.data;
    
    from.data=nullptr;
    from.w=from.h=from.c=0;
    
    return *this;
    }
  
  // pixel access
  
  int pixel_address(int x, int y, int ch) const
    {
    return ch*w*h + y*w + x;
    }
  
  float& operator()(int x, int y, int ch)
    {
    assert(ch<c && ch>=0 && x<w && x>=0 && y<h && y>=0 && "access out of bounds");
    return data[pixel_address(x,y,ch)];
    }
  
  float& operator()(int x, int y)
    {
    assert(c==1 && x<w && x>=0 && y<h && y>=0 && "access out of bounds");
    return data[pixel_address(x,y,0)];
    }
  
  const float& operator()(int x, int y, int ch) const 
    {
    //printf("%d %d %d   %d %d %d\n",w,h,c,x,y,ch);
    assert(ch<c && ch>=0 && x<w && x>=0 && y<h && y>=0 && "access out of bounds");
    return data[pixel_address(x,y,ch)];
    }
  
  const float& operator()(int x, int y) const 
    {
    assert(c==1 && x<w && x>=0 && y<h && y>=0 && "access out of bounds");
    return data[pixel_address(x,y,0)];
    }
  
        float& pixel(int x, int y, int ch)       { return operator()(x,y,ch); }
  const float& pixel(int x, int y, int ch) const { return operator()(x,y,ch); }
        float& pixel(int x, int y)               { return operator()(x,y); }
  const float& pixel(int x, int y) const         { return operator()(x,y); }
  
  
  float clamped_pixel(int x, int y, int ch) const
    {
    assert(ch<c && ch>=0);
    if(x>=w)x=w-1;
    if(y>=h)y=h-1;
    if(x<0)x=0;
    if(y<0)y=0;
    return pixel(x,y,ch);
    }
  
  float clamped_pixel(int x, int y) const
    {
    assert(c==1);
    return clamped_pixel(x,y,0);
    }
  
  void set_pixel(int x, int y, int ch, float v)
    {
    assert(ch<c && ch>=0);
    if(x>=w)return;
    if(y>=h)return;
    if(x<0)return;
    if(y<0)return;
    pixel(x,y,ch)=v;
    }
  
  const float* RowPtr(int row, int channel) const { return data+channel*w*h+row*w; }
        float* RowPtr(int row, int channel)       { return data+channel*w*h+row*w; }
  
  bool contains(float x, float y) const { return x>-0.5f && x<w-0.5f && y>-0.5f && y<h-0.5f; }
  
  bool is_empty(int x, int y) const
    {
    assert(x<w && x>=0 && y<h && y>=0);
    for(int q1=0;q1<c;q1++)if(pixel(x,y,q1))return false;
    return true;
    }
  
  bool is_nonempty_patch(int x, int y, int w=0) const
    {
    for(int q1=x-w;q1<=x+w;q1++)for(int q2=y-w;q2<=y+w;q2++)
      {
      int c1=0;
      for(int ch=0;ch<c;ch++)if(clamped_pixel(q1,q2,ch))c1++;
      if(c1==0)return false;
      }
    return true;
    }
  
  int size(void) const { return w*h*c; }
  
  void clear(void) const { memset(data,0,sizeof(float)*c*w*h); }
  
  // member functions for inexact access
  
  float pixel_nearest (float x, float y, int c) const;
  float pixel_bilinear(float x, float y, int c) const;
  
  // member functions for modifying image
  void l1_normalize(void);
  void RGBtoHSV(void);
  void HSVtoRGB(void);
  void LCHtoRGB(void);
  void RGBtoLCH(void);
  void shift(int c, float v);
  void scale(int c, float v);
  void threshold(float thres);
  void clamp(void);
  void feature_normalize(void);
  void feature_normalize_total(void);
  
  
  // Member functions for slicing an image
  Image get_channel(int c) const;
  void set_channel(int c,const Image& im);
  
  // Member functions for transforming an image
  Image rgb_to_grayscale(void) const;
  Image transpose(void) const;
  Image abs(void) const;
  
  // Image I/O member functions
  void save_binary(const string& filename) const;
  void load_binary(const string& filename);
  
  void load_image  (const string& filename);
  void save_png    (const string& filename) const;
  void save_image  (const string& filename) const;
  
  };



// A 2d point.
// float x, y: the coordinates of the point.
struct Point 
  {
  double x, y;
  
  Point() : x(0), y(0) {}
  Point(double x, double y) : x(x), y(y) {}
  };

// A descriptor for a point in an image.
// point p: x,y coordinates of the image pixel.
// vector<float> data: the descriptor for the pixel.
struct Descriptor
  {
  Point p;
  vector<float> data;
  
  Descriptor(){}
  Descriptor(const Point& p) : p(p) {}
  };

// A match between two points in an Image.
// const Descriptor* a, b: Pointers to the Descriptors in the corresponding images.
// float distance: the distance between the descriptors for the points.
struct Match
  {
  const Descriptor* a=nullptr;
  const Descriptor* b=nullptr;
  float distance=0.f;
  
  Match(){}
  Match(const Descriptor* a,const Descriptor* b,float dist=0.f) : a(a), b(b), distance(dist) {}
  
  bool operator<(const Match& other) { return distance<other.distance; }
  };





// Image I/O functions
inline Image load_binary (const string& filename) { Image im; im.load_binary(filename); return im; }
inline Image load_image  (const string& filename) { Image im; im.load_image(filename);  return im; }
inline void  save_png    (const Image& im, const string& filename) { im.save_png   (filename); }
inline void  save_image  (const Image& im, const string& filename) { im.save_image (filename); }
inline void  save_binary (const Image& im, const string& filename) { im.save_binary(filename); }


// Basic operations
Image rgb_to_grayscale(const Image& im);
Image grayscale_to_rgb(const Image& im, float r, float g, float b);
void rgb_to_hsv(Image& im);
void hsv_to_rgb(Image& im);
void lch_to_rgb(Image& im);
void rgb_to_lch(Image& im);
void shift_image(Image& im, int c, float v);
void scale_image(Image& im, int c, float v);
void clamp_image(Image& im);

// Resizing
Image nearest_resize (const Image& im, int w, int h);
Image bilinear_resize(const Image& im, int w, int h);



// Filtering
Image convolve_image(const Image& im, const Image& filter, bool preserve);
Image make_box_filter(int w);
Image make_highpass_filter(void);
Image make_sharpen_filter(void);
Image make_emboss_filter(void);
Image make_gaussian_filter(float sigma);
Image make_gx_filter(void);
Image make_gy_filter(void);
void feature_normalize(Image& im);
void feature_normalize_total(Image& im);

void threshold_image(Image& im, float thresh);
pair<Image,Image> sobel_image(const Image&  im);
Image colorize_sobel(const Image&  im);
Image smooth_image(const Image&  im, float sigma);
Image bilateral_filter(const Image& im, float sigma, float sigma2);



// Image manipulation
Image get_channel(const Image& im, int c);
bool operator ==(const Image& a, const Image& b);
int same_image(const Image& a, const Image& b);

Image sub_image(const Image& a, const Image& b);
Image add_image(const Image& a, const Image& b);
Image operator-(const Image& a, const Image& b);
Image operator+(const Image& a, const Image& b);


// Harris and panorama
Image structure_matrix(const Image& im, float sigma);
Image cornerness_response(const Image& S, int method);
Image nms_image(const Image& im, int w);
vector<Descriptor> detect_corners(const Image& im, const Image& nms, float thresh, int window);
vector<Descriptor> harris_corner_detector(const Image& im, float sigma, float thresh, int window, int nms, int corner_method);
Image detect_and_draw_corners(const Image& im, float sigma, float thresh, int window, int nms, int corner_method);
Image mark_corners(const Image& im, const vector<Descriptor>& d);

// Panorama
Image both_images(const Image& a, const Image& b);
Image draw_matches(const Image& a, const Image& b, const vector<Match>& matches, const vector<Match>&  inliers);
Image draw_inliers(const Image& a, const Image& b, const Matrix& H, const vector<Match>& m, float thresh);
Image find_and_draw_matches(const Image& a, const Image& b, float sigma, float thresh, int window, int nms, int corner_method);
float l1_distance(const vector<float>& a,const vector<float>& b);
vector<Match> match_descriptors(const vector<Descriptor>& a,const vector<Descriptor>& b);
Point project_point(const Matrix& H, const Point& p);
double point_distance(const Point& p, const Point& q);
vector<Match> model_inliers(const Matrix& H, const vector<Match>& m, float thresh);
void randomize_matches(vector<Match>& m);
Matrix compute_homography_ba(const vector<Match>& matches);
Matrix RANSAC(vector<Match> m, float thresh, int k, int cutoff);
Image combine_images(const Image& a, const Image& b, const Matrix& Hba, float acoeff);
Image panorama_image(const Image& a, const Image& b, float sigma, int corner_method, float thresh, int window, int nms, float inlier_thresh, int iters, int cutoff, float acoeff);
Image cylindrical_project(const Image& im, float f);
Image spherical_project(const Image& im, float f);
