#include <pangolin/pangolin.h>
#include <pangolin/display/image_view.h>

#include <thread>

using namespace  std;

#include "../image.h"

#include "pango-utils.h"

using namespace std;

int main(int argc, char** argv)
  {
  string file="pano/rainier/Rainier1.png";
  
  if(argc==2)file=argv[1];
  
  // Create OpenGL window in single line
  pangolin::CreateWindowAndBind("Main",1280,720);
  
  const int UI_WIDTH = 200;
  
  // Add named Panel and bind to variables beginning 'ui'
  // A Panel is just a View with a default layout and input handling
  pangolin::CreatePanel("ui").SetBounds(0.0, 1.0, 0.0, pangolin::Attach::Pix(UI_WIDTH));
      
  pangolin::View& container = pangolin::Display("container")
                                       .SetLayout(pangolin::LayoutEqual)
                                       .SetBounds(0,1,pangolin::Attach::Pix(UI_WIDTH),1);
  
  
  Image image;
  image=load_image(file);
  
  Image cyl;
  Image spherical;
  
  vector<pangolin::ImageView> iv(3);
  for(pangolin::ImageView& e1:iv)container.AddDisplay(e1);
  
  
  // Safe and efficient binding of named variables.
  pangolin::Var<float> focal_length_pix("ui.focal_length_pix",500,10,10000,true);
  pangolin::Var<std::function<void(void)>>("ui.SaveScreenshot", [&](void){ save_png(download_framebuffer(UI_WIDTH),"output/screenshot"); });
  
  auto run_pipeline=[&](bool force=false)
    {
    //TIME(1);
    bool recompute=force;
    
    recompute|=focal_length_pix.GuiChanged();
    
    if(recompute)
      {
      thread th1([&](){cyl=cylindrical_project(image,focal_length_pix);});
      thread th2([&](){spherical=spherical_project(image,focal_length_pix);});
      th1.join();th2.join();
      }
    
    
    
    // Update Images
    iv[0].SetImage(toPangolin(image));
    iv[1].SetImage(toPangolin(cyl));
    iv[2].SetImage(toPangolin(spherical));
    };
  
  run_pipeline(true);
  
  // Default hooks for exiting (Esc) and fullscreen (tab).
  while(!pangolin::ShouldQuit())
    {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3f(1.0,1.0,1.0);
    
    if(pangolin::GuiVarHasChanged())
      run_pipeline();
    
    
    auto draw_overlay=[&](pangolin::View& view)
      {
      };
    
    iv[0].SetDrawFunction(draw_overlay);
    
    pangolin::FinishFrame();
    }
  
  return 0;
  }
