#include "../image.h"
#include "../utils.h"

#include <string>

using namespace std;

void test_nn_resize()
  {
  Image im = load_image("data/dogsmall.jpg");
  Image resized = nearest_resize(im, im.w*4, im.h*4);
  Image gt = load_image("data/dog4x-nn-for-test.png");
  TEST(same_image(resized, gt));
  
  Image im2 = load_image("data/dog.jpg");
  Image resized2 = nearest_resize(im2, 713, 467);
  Image gt2 = load_image("data/dog-resize-nn.png");
  TEST(same_image(resized2, gt2));
  }

void test_bl_resize()
  {
  Image im = load_image("data/dogsmall.jpg");
  Image resized = bilinear_resize(im, im.w*4, im.h*4);
  Image gt = load_image("data/dog4x-bl.png");
  TEST(same_image(resized, gt));

  Image im2 = load_image("data/dog.jpg");
  Image resized2 = bilinear_resize(im2, 713, 467);
  Image gt2 = load_image("data/dog-resize-bil.png");
  TEST(same_image(resized2, gt2));
  }

void test_multiple_resize()
  {
  Image im = load_image("data/dog.jpg");
  for (int i = 0; i < 10; i++)
    {
    Image im1 = bilinear_resize(im, im.w*4, im.h*4);
    Image im2 = bilinear_resize(im1, im1.w/4, im1.h/4);
    im = im2;
    }
  Image gt = load_image("data/dog-multipleresize.png");
  TEST(same_image(im, gt));
  }


void test_highpass_filter()
  {
  Image im = load_image("data/dog.jpg");
  Image f = make_highpass_filter();
  Image blur = convolve_image(im, f, false);
  blur.clamp();
  Image gt = load_image("data/dog-highpass.png");
  TEST(same_image(blur, gt));
  }

void test_emboss_filter()
  {
  Image im = load_image("data/dog.jpg");
  Image f = make_emboss_filter();
  Image blur = convolve_image(im, f, true);
  blur.clamp();
  
  Image gt = load_image("data/dog-emboss.png");
  TEST(same_image(blur, gt));
  }

void test_sharpen_filter()
  {
  Image im = load_image("data/dog.jpg");
  Image f = make_sharpen_filter();
  Image blur = convolve_image(im, f, true);
  blur.clamp();
  
  Image gt = load_image("data/dog-sharpen.png");
  TEST(same_image(blur, gt));
  }

void test_convolution()
  {
  Image im = load_image("data/dog.jpg");
  Image f = make_box_filter(7);
  Image blur = convolve_image(im, f, true);
  blur.clamp();
  
  Image gt = load_image("data/dog-box7.png");
  TEST(same_image(blur, gt));
  }

void test_gaussian_filter()
  {
  Image f = make_gaussian_filter(7);
  
  for(int i = 0; i < f.w * f.h * f.c; i++)f.data[i] *= 100;
  
  Image gt = load_image("data/gaussian_filter_7.png");
  TEST(same_image(f, gt));
  }

void test_gaussian_blur()
  {
  Image im = load_image("data/dog.jpg");
  Image f = make_gaussian_filter(2);
  Image blur = convolve_image(im, f, true);
  blur.clamp();
  
  Image gt = load_image("data/dog-gauss2.png");
  TEST(same_image(blur, gt));
  }

void test_hybrid_image()
  {
  Image man = load_image("data/melisa.png");
  Image woman = load_image("data/aria.png");
  Image f = make_gaussian_filter(2);
  Image lfreq_man = convolve_image(man, f, true);
  Image lfreq_w = convolve_image(woman, f, true);
  Image hfreq_w = woman - lfreq_w;
  Image reconstruct = lfreq_man + hfreq_w;
  Image gt = load_image("data/hybrid.png");
  reconstruct.clamp();
  TEST(same_image(reconstruct, gt));
  }

void test_frequency_image()
  {
  Image im = load_image("data/dog.jpg");
  Image f = make_gaussian_filter(2);
  Image lfreq = convolve_image(im, f, true);
  Image hfreq = im - lfreq;
  Image reconstruct = lfreq + hfreq;
  
  Image low_freq = load_image("data/low-frequency.png");
  Image high_freq = load_image("data/high-frequency-clamp.png");
  
  lfreq.clamp();
  hfreq.clamp();
  TEST(same_image(lfreq, low_freq));
  TEST(same_image(hfreq, high_freq));
  TEST(same_image(reconstruct, im));
  }

void test_sobel()
  {
  Image im = load_image("data/dog.jpg");
  pair<Image,Image> res = sobel_image(im);
  Image mag = res.first;
  Image theta = res.second;
  
  feature_normalize(mag);
  feature_normalize(theta);
  
  Image gt_mag = load_image("data/magnitude.png");
  Image gt_theta = load_image("data/theta.png");
  TEST(gt_mag.w == mag.w && gt_theta.w == theta.w);
  TEST(gt_mag.h == mag.h && gt_theta.h == theta.h);
  TEST(gt_mag.c == mag.c && gt_theta.c == theta.c);
  if( gt_mag.w != mag.w || gt_theta.w != theta.w || 
      gt_mag.h != mag.h || gt_theta.h != theta.h || 
      gt_mag.c != mag.c || gt_theta.c != theta.c ) return;
  
  for(int i = 0; i < gt_mag.w*gt_mag.h; ++i){
      if(within_eps(gt_mag.data[i], 0)){
          gt_theta.data[i] = 0;
          theta.data[i] = 0;
      }
      if(within_eps(gt_theta.data[i], 0) || within_eps(gt_theta.data[i], 1)){
          gt_theta.data[i] = 0;
          theta.data[i] = 0;
      }
  }
  
  save_png(mag,"output/mag");
  save_png(theta,"output/theta");
  
  Image imo=colorize_sobel(im);
  save_png(imo,"output/color_sobel");
  
  TEST(same_image(mag, gt_mag));
  TEST(same_image(theta, gt_theta));
  }

void test_bilateral()
  {
  Image im = load_image("data/dog.jpg");
  Image bif= bilateral_filter(im,3,0.1);
  
  save_png(bif,"output/bilateral");
  Image gt = load_image("data/dog-bilateral.png");
  TEST(same_image(bif, gt));
  }

void run_tests()
  {
  test_nn_resize();
  test_bl_resize();
  test_multiple_resize();
  
  test_gaussian_filter();
  test_sharpen_filter();
  test_emboss_filter();
  test_highpass_filter();
  test_convolution();
  test_gaussian_blur();
  test_hybrid_image();
  test_frequency_image();
  test_sobel();
  
  test_bilateral();
  printf("%d tests, %d passed, %d failed\n", tests_total, tests_total-tests_fail, tests_fail);
  }

int main(int argc, char **argv)
  {
  run_tests();
  
  //test_bilateral();
  return 0;
  }
