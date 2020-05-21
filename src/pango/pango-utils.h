#pragma once

#include <pangolin/pangolin.h>
#include "../image.h"

// Pangolin uses HWC instead of CHW
inline pangolin::TypedImage toPangolin(const Image& im)
  {
  //TIME(1);
  assert(im.c==1 || im.c==3);
  //pangolin::PixelFormat fmt=im.c==1?pangolin::PixelFormatFromString("GRAY8")
  //                                 :pangolin::PixelFormatFromString("RGB24");
  //pangolin::PixelFormat fmt=im.c==1?pangolin::PixelFormatFromString("GRAY32F")
  //                                 :pangolin::PixelFormatFromString("RGB96F");
  
  
  auto fmt=pangolin::PixelFormatFromString("RGB24");
  
  pangolin::TypedImage a(im.w,im.h,fmt);
  
  unsigned char*data=(unsigned char*)a.ptr;
  
  for(int q2=0;q2<im.h;q2++)for(int q1=0;q1<im.w;q1++)for(int c=0;c<im.c;c++)for(int q3=0;q3<3/im.c;q3++)
    *(data++)=(unsigned char)(im(q1,q2,c)*255.0);
  
  return a;
  }

inline Image feat_norm(const Image& a){ Image b=a; feature_normalize_total(b);return b; }

inline Image download_framebuffer(int UI)
  {
  //printf("%d %d\n",pangolin::Display("container").v.w,pangolin::Display("container").v.h);
  int sizex=pangolin::Display("container").v.w+UI;
  int sizey=pangolin::Display("container").v.h;
  
  unsigned char* im2=(unsigned char*)malloc(sizex*sizey*4);
  unsigned char* im=(unsigned char*)malloc(sizex*sizey*4);
  unsigned char* imj=(unsigned char*)malloc(sizex*sizey*3);
  glPixelStorei(GL_PACK_ALIGNMENT,1);
  glReadPixels(0,0,sizex,sizey,GL_RGBA,GL_UNSIGNED_BYTE,im2);     
  for(int q1=0;q1<sizey;q1++)memcpy(im+sizex*4*q1,im2+sizex*4*(sizey-1-q1),sizex*4);
  for(int q1=0;q1<sizex*sizey;q1++)im[q1*4+3]=255;
  for(int q1=0;q1<sizey;q1++)for(int q2=0;q2<sizex;q2++)memcpy(imj+(q1*sizex+q2)*3,im+(q1*sizex+q2)*4,3);
  
  
  Image i(sizex,sizey,3);
  for(int q2=0;q2<sizey;q2++)for(int q1=0;q1<sizex;q1++)
    for(int q3=0;q3<3;q3++)i(q1,q2,q3)=im[q2*sizex*4+q1*4+q3]/255.0;
  
  free(im2);
  free(im);
  free(imj);
  return i;
  }
