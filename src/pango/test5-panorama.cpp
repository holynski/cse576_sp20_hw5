#include <pangolin/pangolin.h>
#include <pangolin/display/image_view.h>

#include <thread>

#include "../image.h"
#include "pango-utils.h"

using namespace std;


struct Data
  {
  Image input;
  Image image;
  Image structure;
  Image response;
  Image nms;
  vector<Descriptor> desc;
  
  void process(int proj, float focal,float sigma, int nms_rad, float thresh_corner, int window, int corner_method)
    {
    if(proj==0)image=input;
    if(proj==1)image=cylindrical_project(input,focal);
    if(proj==2)image=spherical_project(input,focal);
    structure = structure_matrix(image,sigma);
    response = cornerness_response(structure,corner_method);
    nms = nms_image(response,nms_rad);
    desc = detect_corners(image,nms,thresh_corner,window);
    }
  };

int main(int argc, char** argv)
  {
  const string file1=argc==3?argv[1]:"pano/rainier/Rainier1.png";
  const string file2=argc==3?argv[2]:"pano/rainier/Rainier2.png";
  
  // Create OpenGL window in single line
  pangolin::CreateWindowAndBind("Main",1280,720);
  
  const int UI_WIDTH = 210;
  
  // Add named Panel and bind to variables beginning 'ui'
  // A Panel is just a View with a default layout and input handling
  pangolin::CreatePanel("ui").SetBounds(0.0, 1.0, 0.0, pangolin::Attach::Pix(UI_WIDTH));
      
  pangolin::View& container = pangolin::Display("container")
                                       .SetLayout(pangolin::LayoutEqual)
                                       .SetBounds(0,1,pangolin::Attach::Pix(UI_WIDTH),1);
  
  Data d1,d2;
  Image combined;
  vector<Match> match;
  vector<Match> homogMatch;
  vector<Match> inliers;
  
  //image=load_image("data/dogbw.png");
  d1.input=load_image(file1);
  d2.input=load_image(file2);
  
  vector<pangolin::ImageView> iv(2);
  for(pangolin::ImageView& e1:iv)container.AddDisplay(e1);
  
  
  // Safe and efficient binding of named variables.
  pangolin::Var<int>("ui.planar",0,0,0);
  pangolin::Var<int>("ui.cylindrical",1,1,1);
  pangolin::Var<int>("ui.spherical",2,2,2);
  pangolin::Var<int> projection_type("ui.proj_type_0_1_2",0,0,2);
  pangolin::Var<float> focal("ui.focal_length",1000,100,3000);
  pangolin::Var<float> structure_blur("ui.structure_blur",2,0,7);
  pangolin::Var<int> nms_rad("ui.nms_rad",7,1,50,true);
  pangolin::Var<int> descriptor_window("ui.descriptor_window",7,5,20);
  pangolin::Var<bool> corner_method("ui.exact2ndEV",false,0,1);
  pangolin::Var<float> thresh_corner("ui.thresh_corner",0.1,1e-2,1e1,true);
  pangolin::Var<bool> show_corners("ui.show_corners",true,0,1);
  pangolin::Var<bool> show_matches("ui.show_matches",true,0,1);
  pangolin::Var<int>("ui.RANSAC_PARAMS",0,0,0);
  pangolin::Var<float> thresh_inliers_pix("ui.thresh_inliers_pix",5,0.1,100,true);
  pangolin::Var<int> iterations_ransac("ui.iters_ransac",1000,1,30000,true);
  pangolin::Var<int> cutoff_min_inliers("ui.cutoff_inliers",50,1,1000,true);
  
  pangolin::Var<bool> show_matches_homog("ui.show_matches_homog",true,0,1);
  pangolin::Var<bool> show_inliers("ui.show_inliers",true,0,1);
  pangolin::Var<bool> show_combined("ui.show_combined",true,0,1);
  pangolin::Var<float> blend_alpha("ui.blend_alpha",0.5,0,1);
  
  pangolin::Var<std::function<void(void)>>("ui.SingleShuffleHomog",[&]()
    {
    vector<Match> m1=match;
    randomize_matches(m1);
    if((int)m1.size()<4)
      {
      printf("Not enough matches for homography\n");
      return;
      }
    
    homogMatch=vector<Match>(&m1[0],&m1[4]);
    
    //printf("%zu %f %p %p\n",m1.size(),m2[0].distance,m2[0].a,m2[0].b);
    Matrix Hba=compute_homography_ba(homogMatch);
    //Hba.print();
    
    inliers=model_inliers(Hba,match,thresh_inliers_pix);
    
    combined=Image(100,100);
    if(show_combined)combined=combine_images(d1.image,d2.image,Hba,blend_alpha);
    iv[1].SetImage(toPangolin(combined));
    });
  
  pangolin::Var<bool> auto_RANSAC("ui.auto_RANSAC",true,0,1);
  
  
  function<void(void)> runRANSAC=[&]()
    {
    Matrix Hba=RANSAC(match,thresh_inliers_pix,iterations_ransac,cutoff_min_inliers);
    //Hba.print();
    homogMatch.clear();
    inliers=model_inliers(Hba,match,thresh_inliers_pix);
    
    combined=Image(100,100);
    if(show_combined)combined=combine_images(d1.image,d2.image,Hba,blend_alpha);
    iv[1].SetImage(toPangolin(combined));
    };
  
  pangolin::Var<std::function<void(void)>>("ui.Run_RANSAC", runRANSAC);
  pangolin::Var<string> filename("ui.filename","output");
  pangolin::Var<std::function<void(void)>>("ui.SaveImage", [&](void){ save_png(combined,string("output/")+string(filename)); });
  pangolin::Var<std::function<void(void)>>("ui.SaveScreenshot", [&](void){ save_png(download_framebuffer(UI_WIDTH),"output/screenshot"); });
  
  auto run_pipeline=[&](bool force=false)
    {
    //TIME(1);
    bool recompute=force;
    
    recompute|=projection_type.GuiChanged();
    recompute|=focal.GuiChanged();
    recompute|=structure_blur.GuiChanged();
    recompute|=nms_rad.GuiChanged();
    recompute|=descriptor_window.GuiChanged();
    recompute|=thresh_corner.GuiChanged();
    recompute|=corner_method.GuiChanged();
    
    if(recompute)
      {
      thread th1([&](){d1.process(projection_type,focal,structure_blur,nms_rad,thresh_corner,descriptor_window,corner_method);});
      thread th2([&](){d2.process(projection_type,focal,structure_blur,nms_rad,thresh_corner,descriptor_window,corner_method);});
      th1.join();
      th2.join();
      
      match=match_descriptors(d1.desc,d2.desc);
      inliers.clear();
      homogMatch.clear();
      
      if(auto_RANSAC)runRANSAC();
      // Update Images
      iv[0].SetImage(toPangolin(both_images(d1.image,d2.image)));
      }
    else
      {
      recompute|=thresh_inliers_pix.GuiChanged();
      recompute|=iterations_ransac.GuiChanged();
      recompute|=cutoff_min_inliers.GuiChanged();
      recompute|=blend_alpha.GuiChanged();
      
      if(auto_RANSAC && recompute)runRANSAC();
      }
    
    
    };
  
  run_pipeline(true);
  
  // Default hooks for exiting (Esc) and fullscreen (tab).
  while(!pangolin::ShouldQuit())
    {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3f(1.0,1.0,1.0);
    
    run_pipeline();
    
    //display the image
    //image1.Activate();
    
    auto draw_overlay=[&](pangolin::View& view)
      {
      view.Activate();
      
      int off=d1.image.w;
      
      if(show_corners)
        {
        glColor3d(1,0,1);glLineWidth(2);
        for(const Descriptor&d:d1.desc)pangolin::glDrawCross(d.p.x,d.p.y,5);
        for(const Descriptor&d:d2.desc)pangolin::glDrawCross(d.p.x+off,d.p.y,5);
        }
      
      if(show_matches)
        {
        glColor3d(1,0,0);glLineWidth(1);
        for(const Match&m:match)pangolin::glDrawLine(m.a->p.x,m.a->p.y,m.b->p.x+off,m.b->p.y);
        }
      
      if(show_inliers)
        {
        glLineWidth(2);
        for(const Match&m:inliers)
          {
          glColor3d(0,1,1);
          pangolin::glDrawLine(m.a->p.x,m.a->p.y,m.b->p.x+off,m.b->p.y);
          glColor3d(0,1,0);
          float size=4;
          pangolin::glDrawRectPerimeter(m.a->p.x-size,m.a->p.y-size,m.a->p.x+size,m.a->p.y+size);
          pangolin::glDrawRectPerimeter(m.b->p.x+off-size,m.b->p.y-size,m.b->p.x+size+off,m.b->p.y+size);
          }
        }
      
      if(show_matches_homog)
        {
        glLineWidth(3);
        for(const Match&m:homogMatch)
          {
          glColor3d(1,1,0);
          pangolin::glDrawLine(m.a->p.x,m.a->p.y,m.b->p.x+off,m.b->p.y);
          glColor3d(1,0,1);
          float size=4;
          pangolin::glDrawRectPerimeter(m.a->p.x-size,m.a->p.y-size,m.a->p.x+size,m.a->p.y+size);
          pangolin::glDrawRectPerimeter(m.b->p.x+off-size,m.b->p.y-size,m.b->p.x+size+off,m.b->p.y+size);
          }
        }
      
      };
    
    iv[0].SetDrawFunction(draw_overlay);
    
    pangolin::FinishFrame();
    }
  
  return 0;
  }
