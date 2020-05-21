#include <pangolin/pangolin.h>
#include <pangolin/display/image_view.h>

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
  Image structure;
  Image response;
  Image nms;
  vector<Descriptor> desc;
  
  //image=load_image("data/dogbw.png");
  image=load_image(file);
  
  vector<pangolin::ImageView> iv(3);
  for(pangolin::ImageView& e1:iv)container.AddDisplay(e1);
  
  
  // Safe and efficient binding of named variables.
  pangolin::Var<float> structure_blur("ui.structure_blur",2,0,5);
  pangolin::Var<int> nms_rad("ui.nms_rad",3,1,50,true);
  pangolin::Var<int> descriptor_window("ui.descriptor_window",5,5,20);
  pangolin::Var<bool> corner_method("ui.exact2ndEV",false,0,1);
  pangolin::Var<float> thresh_corner("ui.thresh_corner",0.2,1e-2,1e1,true);
  pangolin::Var<bool> show_nms("ui.show_nms",false,0,1);
  pangolin::Var<bool> show_corners("ui.show_corners",true,0,1);
  pangolin::Var<bool> show_nms_response("ui.show_nms(response)",false,0,1);
  pangolin::Var<bool> show_corners_response("ui.show_corners(response)",false,0,1);
  pangolin::Var<std::function<void(void)>>("ui.SaveScreenshot", [&](void){ save_png(download_framebuffer(UI_WIDTH),"output/screenshot"); });
  
  auto run_pipeline=[&]()
    {
    
    structure = structure_matrix(image,structure_blur);
    response = cornerness_response(structure,corner_method);
    nms = nms_image(response,nms_rad);
    desc = detect_corners(image,nms,thresh_corner,descriptor_window);
    
    // Update Images
    iv[0].SetImage(toPangolin(image));
    iv[1].SetImage(toPangolin(feat_norm(structure)));
    iv[2].SetImage(toPangolin(feat_norm(response)));
    };
  
  run_pipeline();
  
  // Default hooks for exiting (Esc) and fullscreen (tab).
  while(!pangolin::ShouldQuit())
    {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3f(1.0,1.0,1.0);
    
    if(pangolin::GuiVarHasChanged())
      run_pipeline();
    
    //display the image
    //image1.Activate();
    
    auto draw_overlay=[&](pangolin::View& view)
      {
      view.Activate();
      if(show_nms)
        {
        glColor3d(0,1,0);glLineWidth(1);
        for(int q2=0;q2<nms.h;q2++)for(int q1=0;q1<nms.w;q1++)if(nms(q1,q2)>0)
          pangolin::glDrawCross(q1,q2,5);
        }
      if(show_corners)
        {
        glColor3d(1,0,1);glLineWidth(2);
        for(Descriptor&d:desc)
          pangolin::glDrawCross(d.p.x,d.p.y,5);
        }
      };
    
    auto draw_overlay_response=[&](pangolin::View& view)
      {
      view.Activate();
      if(show_nms_response)
        {
        glColor4d(0,1,0,0.5);glLineWidth(0.5);
        for(int q2=0;q2<nms.h;q2++)for(int q1=0;q1<nms.w;q1++)if(nms(q1,q2)>0)
          pangolin::glDrawCross(q1,q2,2);
        }
      if(show_corners_response)
        {
        glColor4d(1,0,1,0.5);glLineWidth(1);
        for(Descriptor&d:desc)
          pangolin::glDrawCross(d.p.x,d.p.y,3);
        }
      };
    
    iv[0].SetDrawFunction(draw_overlay);
    iv[2].SetDrawFunction(draw_overlay_response);
    
    pangolin::FinishFrame();
    }
  
  return 0;
  }
