#include "../image.h"
#include "../utils.h"
#include "../matrix.h"

#include <string>
#include <thread>
#include <map>
#include <mutex>

using namespace std;

struct image_map
  {
  map<string,Image> im;
  mutex m;
  string outdir,indir;
  Image& operator[](const string& a){lock_guard<mutex> LG(m); return im[a];}
  };

void save_images(const image_map& im,const string& out)
  {
  TIME(1);
  vector < thread > th;
  for(auto&e1:im.im)//if(e1.first.find("-")!=string::npos)
  th.emplace_back([&]()
    {
    save_png(e1.second,out+e1.first);
    printf("%s saved!\n",(out+e1.first).c_str());
    });
  for(auto&e1:th)e1.join();
  }

// PROJ_METHOD:    0 - Identity,    1 - cylindrical,    2 - spherical
void load_images(image_map& im, const string& indir, const string& outdir, int numpics, int PROJ_METHOD=0, double FOCAL_LEN=1000)
  {
  TIME(1);
  im.indir=indir;
  im.outdir=outdir;
  vector < unique_ptr<thread> > th; 
  for(int q1=0;q1<numpics;q1++)th.emplace_back(new thread([&,q1]()
    {
    string file=indir+to_string(q1)+".jpg";
    Image in=load_image(file);
    if(PROJ_METHOD==0)im[to_string(q1)]=in;
    if(PROJ_METHOD==1)im[to_string(q1)]=cylindrical_project(in,FOCAL_LEN);
    if(PROJ_METHOD==2)im[to_string(q1)]=spherical_project(in,FOCAL_LEN);
    printf("%s loaded into im[\"%s\"]\n",file.c_str(),to_string(q1).c_str());
    }));
  for(auto&e1:th)e1->join();th.clear();
  }

void create_panorama(image_map& im, const string& out, const string& aname, const string& bname,
                     float sigma, int corner_method, float thresh, int window, int nms, float inlier_thresh, int iters, int cutoff, float acoeff)
  {
  printf("Combining %s and %s into %s...\n",aname.c_str(),bname.c_str(),out.c_str());
  assert(im[aname].size()!=0 && "Image A invalid\n");
  assert(im[bname].size()!=0 && "Image B invalid\n");
  im[out]=panorama_image(im[aname],im[bname],sigma,corner_method,thresh,window,nms,inlier_thresh,iters,cutoff,acoeff);
  save_png(im[out],im.outdir+out);
  printf("%s finished computing\n",out.c_str());
  }

// HW5 5
void do_columbia_peak(void)
  {
  string indir="pano/columbia/";
  string outdir="output/columbia/";
  
  image_map im;
  load_images(im,indir,outdir,11,2,1310);  // im, dir, numpics (0..numpics-1),  PROJ_METHOD,  FOCAL_LEN
  
  vector < thread > th; 
  
  th.push_back(thread([&](){create_panorama(im,"0-1","0","1" ,2,0,0.05,7,7,5,50000,100,0.5);}));
  th.push_back(thread([&](){create_panorama(im,"2-3","2","3" ,2,0,0.05,7,7,5,50000,100,0.5);}));
  th.push_back(thread([&](){create_panorama(im,"4-5","4","5" ,2,0,0.05,7,7,5,50000,100,0.5);}));
  th.push_back(thread([&](){create_panorama(im,"6-7","6","7" ,2,0,0.05,7,7,5,50000,100,0.5);}));
  th.push_back(thread([&](){create_panorama(im,"8-9","8","9" ,2,0,0.05,7,7,5,50000,100,0.5);}));
  for(auto&e1:th)e1.join();th.clear();
  
  
  th.push_back(thread([&](){create_panorama(im,"8--10",  "8-9",  "10"  ,2,0,0.15,11,7,5,50000,100,0.5);}));
  th.push_back(thread([&](){create_panorama(im,"0--3" ,  "0-1",  "2-3" ,2,0,0.04,11,7,5,50000,100,0.5);}));
  th.push_back(thread([&](){create_panorama(im,"4--7" ,  "4-5",  "6-7" ,2,0,0.04,11,7,5,50000,100,0.5);}));
  for(auto&e1:th)e1.join();th.clear();
  
  
  create_panorama(im,"4--10" , "4--7",  "8--10" ,2,0,0.04,10,7,5,50000,100,0.5);
  create_panorama(im,"all"   , "0--3",  "4--10" ,2,0,0.04,10,7,5,50000,100,0.5);
  
  save_images(im,outdir);
  
  }


// HW5 5
void do_rainier(void)
  {
  string indir="pano/rainier/";
  string outdir="output/rainier/";
  
  image_map im;
  load_images(im,indir,outdir,6,0,710);  // im, dir, numpics (0..numpics-1),  PROJ_METHOD,  FOCAL_LEN
  
  create_panorama(im,"0-1","0","1" ,2,0,0.05,7,7,5,50000,100,0.5);
  create_panorama(im,"2-3","2","3" ,2,0,0.05,7,7,5,50000,100,0.5);
  create_panorama(im,"4-5","4","5" ,2,0,0.05,7,7,5,50000,100,0.5);
  
  create_panorama(im,"0--3","0-1","2-3"  ,2,0,0.05,7,7,5,50000,100,0.5);
  create_panorama(im,"all" ,"0--3","4-5" ,2,0,0.05,7,7,5,50000,100,0.5);
  
  save_images(im,outdir);
  
  }


// HW5 5
void do_field(void)
  {
  string indir="pano/field/";
  string outdir="output/field/";
  
  image_map im;
  load_images(im,indir,outdir,8,1,1200);  // im, dir, numpics (0..numpics-1),  PROJ_METHOD,  FOCAL_LEN
  
  vector < thread > th; 
  
  
  th.push_back(thread([&](){create_panorama(im,"2-3","2","3" ,3,0,0.05,11,7,5,50000,100,0);}));
  th.push_back(thread([&](){create_panorama(im,"4-5","4","5" ,3,0,0.05,11,7,5,50000,100,1);}));
  th.push_back(thread([&](){create_panorama(im,"6-7","6","7" ,3,0,0.05,11,7,5,50000,100,1);}));
  for(auto&e1:th)e1.join();th.clear();
  
  
  th.push_back(thread([&](){create_panorama(im,"4--7", "4-5",  "6-7" ,3,0,0.05,11,7,5,50000,100,1);}));
  for(auto&e1:th)e1.join();th.clear();
  
  create_panorama(im,"all"   , "2-3",  "4--7" ,3,0,0.05,11,7,5,50000,100,0.5);
  
  save_images(im,outdir);
  
  }


// HW5 5
void do_helens(void)
  {
  string indir="pano/helens/";
  string outdir="output/helens/";
  
  image_map im;
  load_images(im,indir,outdir,6,1,950);  // im, dir, numpics (0..numpics-1),  PROJ_METHOD,  FOCAL_LEN
  
  vector < thread > th; 
  
  
  th.push_back(thread([&](){create_panorama(im,"0-1","0","1" ,2,0,0.05,11,7,5,150000,100,0.5);}));
  th.push_back(thread([&](){create_panorama(im,"2-3","2","3" ,2,0,0.05,11,7,5,150000,100,0.5);}));
  th.push_back(thread([&](){create_panorama(im,"4-5","4","5" ,2,0,0.05,11,7,5,150000,100,0.5);}));
  for(auto&e1:th)e1.join();th.clear();
  
  
  create_panorama(im,"0--3", "0-1",  "2-3" ,2,0,0.05,11,7,5,50000,100,0.5);
  create_panorama(im,"all" , "0--3", "4-5" ,2,0,0.05,11,7,5,50000,100,0.5);
  
  save_images(im,outdir);
  
  }


// HW5 5
void do_sun(void)
  {
  string indir="pano/sun/";
  string outdir="output/sun/";
  
  image_map im;
  load_images(im,indir,outdir,5,1,1000);  // im, dir, numpics (0..numpics-1),  PROJ_METHOD,  FOCAL_LEN
  
  vector < thread > th; 
  
  
  th.push_back(thread([&](){create_panorama(im,"0-1","0","1" ,2,0,0.05,11,7,5,150000,100,0.5);}));
  th.push_back(thread([&](){create_panorama(im,"2-3","2","3" ,2,0,0.05,11,7,5,150000,100,0.5);}));
  for(auto&e1:th)e1.join();th.clear();
  
  create_panorama(im,"2--4", "2-3",  "4"   ,2,0,0.05,11,7,5,50000,100,0.5);
  create_panorama(im,"all" , "0-1", "2--4" ,2,0,0.05,11,7,5,50000,100,0.5);
  
  save_images(im,outdir);
  
  }




void do_white_wall(void)
  {
  string indir="pano/wall/";
  string outdir="output/wall/";
  
  image_map im;
  load_images(im,indir,outdir,24);  // im, dir, numpics (0..numpics-1),  PROJ_METHOD,  FOCAL_LEN
  
  vector < thread > th; 
  
  
  th.push_back(thread([&]()
    {
    create_panorama(im,"1--2","2"   ,"1" ,2,0,0.05,11,7,5,50000,100,0.5);
    create_panorama(im,"0--2","1--2","0" ,2,0,0.05,11,7,5,50000,100,0.5);
    create_panorama(im,"0--3","0--2","3" ,2,0,0.05,11,7,5,50000,100,0.5);
    create_panorama(im,"0--4","0--3","4" ,2,0,0.05,11,7,5,50000,100,0.5);
    }));
  th.push_back(thread([&]()
    {
    create_panorama(im,"7--8" ,"7"   ,"8"     ,2,0,0.05,11,7,5,50000,100,0.5);
    create_panorama(im,"5--6" ,"6"   ,"5"     ,2,0,0.05,11,7,5,50000,100,0.5);
    create_panorama(im,"9--10","9"   ,"10"    ,2,0,0.05,11,7,5,50000,100,0.5);
    create_panorama(im,"5--8" ,"5--6","7--8"  ,2,0,0.05,11,7,5,50000,100,0.5);
    create_panorama(im,"5--10","5--8","9--10" ,2,0,0.05,11,7,5,50000,100,0.5);
    }));
  th.push_back(thread([&]()
    {
    create_panorama(im,"11--12" ,"12"   ,"11"     ,2,0,0.05,11,7,5,50000,100,0.5);
    create_panorama(im,"13--14" ,"14"   ,"13"     ,2,0,0.05,11,7,5,50000,100,0.5);
    create_panorama(im,"15--16" ,"15"   ,"16"     ,2,0,0.05,11,7,5,50000,100,0.5);
    create_panorama(im,"13--16" ,"13--14","15--16",2,0,0.05,11,7,5,50000,100,0.5);
    create_panorama(im,"11--16" ,"13--16","11--12",2,0,0.05,11,7,5,50000,100,0.5);
    }));
  th.push_back(thread([&]()
    {
    create_panorama(im,"11--12" ,"12"   ,"11"     ,2,0,0.05,11,7,5,50000,100,0.5);
    create_panorama(im,"13--14" ,"14"   ,"13"     ,2,0,0.05,11,7,5,50000,100,0.5);
    create_panorama(im,"15--16" ,"15"   ,"16"     ,2,0,0.05,11,7,5,50000,100,0.5);
    create_panorama(im,"13--16" ,"13--14","15--16",2,0,0.05,11,7,5,50000,100,0.5);
    create_panorama(im,"11--16" ,"13--16","11--12",2,0,0.05,11,7,5,50000,100,0.5);
    }));
  
  th.push_back(thread([&]()
    {
    create_panorama(im,"17--18" ,"18"    ,"17"     ,2,0,0.02,11,7,5,50000,100,0.5);
    create_panorama(im,"19--20" ,"20"    ,"19"     ,2,0,0.02,11,7,5,50000,100,0.5);
    create_panorama(im,"21--22" ,"21"    ,"22"     ,2,0,0.02,11,7,5,50000,100,0.5);
    create_panorama(im,"21--23" ,"21--22","23"     ,2,0,0.02,11,7,5,50000,100,0.5);
    create_panorama(im,"17--20" ,"19--20","17--18" ,2,0,0.02,11,7,5,50000,100,0.5);
    create_panorama(im,"17--23" ,"17--20","21--23" ,2,0,0.02,11,7,5,50000,100,0.5);
    }));
  
  
  
  for(auto&e1:th)e1.join();th.clear();
  
  save_images(im,outdir);
  
  }



void do_cse_glade(void)
  {
  string indir="pano/cse/";
  string outdir="output/cse/";
  
  image_map im;
  load_images(im,indir,outdir,19,2,1310/1.6);  // im, dir, numpics (0..numpics-1),  PROJ_METHOD,  FOCAL_LEN
  
  vector < thread > th; 
  
  th.push_back(thread([&](){create_panorama(im,"1-2","1","2"     ,2,0,0.05,7,7,5,50000,100,0.5);}));
  th.push_back(thread([&](){create_panorama(im,"3-4","3","4"     ,2,0,0.05,7,7,5,50000,100,0.5);}));
  th.push_back(thread([&](){create_panorama(im,"5-6","5","6"     ,2,0,0.05,7,7,5,50000,100,0.5);}));
  th.push_back(thread([&](){create_panorama(im,"7-8","7","8"     ,2,0,0.05,7,7,5,50000,100,0.5);}));
  th.push_back(thread([&](){create_panorama(im,"9-10","9","10"   ,2,0,0.05,7,7,5,50000,100,0.5);}));
  th.push_back(thread([&](){create_panorama(im,"11-12","11","12" ,2,0,0.05,7,7,5,50000,100,0.5);}));
  th.push_back(thread([&](){create_panorama(im,"13-14","13","14" ,2,0,0.05,7,7,5,50000,100,0.5);}));
  th.push_back(thread([&](){create_panorama(im,"15-16","15","16" ,2,0,0.05,7,7,5,50000,100,0.5);}));
  for(auto&e1:th)e1.join();th.clear();
  
  
  
  th.push_back(thread([&](){create_panorama(im,"1--4" ,  "3-4", "1-2"    ,2,0,0.04,11,7,5,50000,100,0.5);}));
  th.push_back(thread([&](){create_panorama(im,"5--8" ,  "7-8",  "5-6"   ,2,0,0.04,11,7,5,50000,100,0.5);}));
  th.push_back(thread([&](){create_panorama(im,"9--12" , "11-12", "9-10" ,2,0,0.04,11,7,5,50000,100,0.5);}));
  th.push_back(thread([&](){create_panorama(im,"13--16" ,"15-16","13-14" ,2,0,0.04,11,7,5,50000,100,0.5);}));
  for(auto&e1:th)e1.join();th.clear();
  
  
  th.push_back(thread([&](){create_panorama(im,"1--8" , "1--4", "5--8" ,2,0,0.04,11,7,5,50000,100,0.5);}));
  th.push_back(thread([&](){create_panorama(im,"9--16" ,"9--12","13--16" ,2,0,0.04,11,7,5,50000,100,0.5);}));
  for(auto&e1:th)e1.join();th.clear();
  
  create_panorama(im,"1--16" , "9--16", "1--8" ,2,0,0.04,11,7,5,50000,100,0.5);
  
  save_images(im,outdir);
  
  }





int main(int argc, char **argv)
  {
  
  if(argc<=1)
    {
    printf("USAGE: ./make-panorama [name]=rainier/columbia/helens/field/sun/wall...\n");
    return 0;
    }
  
  if(string(argv[1])=="columbia")do_columbia_peak();
  if(string(argv[1])=="rainier")do_rainier();
  if(string(argv[1])=="field")do_field();
  if(string(argv[1])=="helens")do_helens();
  if(string(argv[1])=="sun")do_sun();
  if(string(argv[1])=="wall")do_white_wall();
  if(string(argv[1])=="cse")do_cse_glade();
  
  
  
  return 0;
  }
