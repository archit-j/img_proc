#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#define INF 1000000007

using namespace cv;
using namespace std;

vector<Point>  field;
//for the field green
const int resolution_of_whitecount=1;
int iLowH = 25;
int iHighH = 51;

int iLowS = 135; 
int iHighS = 255;

int iLowV = 12;
int iHighV = 106;

int iLowH_line = 27;
int iHighH_line = 62;

int iLowS_line = 100; 
int iHighS_line = 204;

int iLowV_line = 27;
int iHighV_line = 121;


int smooth=1;
int smooth_size=5;

int smooth_line=1;
int smooth_size_line=5;

int countour_field_switch=0;
int field_generate=0;
int region_of_interest_switch=0;
int canny_switch=0;
int blur_median=3;

int canny_thresh=50;
int canny_ratio=3;
int canny_kernel=1;


int amount_of_white_neighbours=2;
int resolution_of_digestion=12;
int radius_of_major_nodes=5;
int nearest_neighbour_scope=15;
int calc_cij_switch=0;
int node_draw_switch=0;
int line_traversal_scope=2;
int Tolerance=5;

int line_cij=0;
int Add_line=0;

void calcCij(Mat &image, vector<vector<int> > &cij) //cij implies information about x's and t's
{
  
  Size size=image.size();
  vector<vector<int> > whiteCount(vector<vector<int> >(size.width, vector<int>(size.height, INF)));
  vector<vector<int> > Digest(vector<vector<int> >(size.width, vector<int>(size.height, INF)));
 // vector<vector<int> > cij(vector<vector<int> >(size.width, vector<int>(size.height, INF)));
//  vector<vector<int> > cij(vector<vector<int> >(size.width, vector<int>(size.height, INF)));

  
  for(int j= resolution_of_whitecount   ;j<size.height- resolution_of_whitecount   ;j++)
  {
    for(int i= resolution_of_whitecount   ;i<size.width- resolution_of_whitecount   ;i++)
    {
      whiteCount[i][j]=0;

      for(int xx=-1*resolution_of_whitecount;xx<resolution_of_whitecount+1;xx++)
      {
        for(int yy=-1*resolution_of_whitecount;yy<resolution_of_whitecount+1;yy++)
        {
          Point pt(i+xx,j+yy);
          Vec3b intensity = image.at<Vec3b>(j+yy, i+xx);
          uchar blue = intensity.val[0];
          uchar green = intensity.val[1];
          uchar red = intensity.val[2];
          if(blue>0||green>0||red>0)
          {
            image.at<Vec3b>(j,i)[0]=0;
            image.at<Vec3b>(j,i)[1]=0;
            image.at<Vec3b>(j,i)[2]=0;
            whiteCount[i][j]++;
          }
        }
      }

      Digest[i][j]=0;

      if(whiteCount[i][j]==amount_of_white_neighbours+1)
      {
        Digest[i][j]=1;
        cij[i][j]=1;

      }
      if(whiteCount[i][j]==amount_of_white_neighbours+2)
      {
        Digest[i][j]=3;
        cij[i][j]=1;

      }
      if(whiteCount[i][j]==amount_of_white_neighbours+3)
      {
        Digest[i][j]=9;
        cij[i][j]=1;

      }
      //
      //
    }
  }

  for(int j= resolution_of_digestion ;j<size.height- resolution_of_digestion ;j++)
  {
    for(int i= resolution_of_digestion ;i<size.width- resolution_of_digestion ;i++)
    {
      if(whiteCount[i][j] == 0||Digest[i][j]==0)
          continue;
      int max_digest=0,maxi=0,maxj=0;
      if(Digest[i][j]>0)
      { max_digest=Digest[i][j];
        maxj=j;
        maxi=i;
        int digested_count = 0;
        for(int xx=-1*resolution_of_digestion ; xx <resolution_of_digestion + 1 ; xx++)
        {
          for(int yy=-1*resolution_of_digestion ; yy <resolution_of_digestion + 1 ; yy++)
          {
           if(Digest[i+xx][j+yy]>max_digest)
            { 
              max_digest=Digest[i+xx][j+yy];
              maxi=i+xx;
              maxj=j+yy;
            }
            else 
            {
              Digest[i+xx][j+yy]=0;
              digested_count++;
            }
          }
        }
        Digest[maxi][maxj]+=digested_count;
        cij[maxi][maxj]=2;
      }
    }
  }
}

void createImageNodes(Mat &image, vector<vector<int> > &cij)
  {
    for (int i = 0; i < cij.size(); ++i)
  {
    for (int j = 0; j < cij[5].size(); ++j)
    { 
      
      { 
        if(cij[i][j] == 1)
        circle(image, Point(i, j), 3, CV_RGB(255, 0, 0));
        if(cij[i][j] ==2)
        circle(image, Point(i,j), radius_of_major_nodes, CV_RGB(0, 255, 0));
      }
    }
  }
}
void createImageEdges(Mat image, vector<vector<int> > &cij)
{ ///loop to traverse the image
  Size size=image.size();
  int IMAGE_WIDTH = size.width;
  int IMAGE_HEIGHT = size.height;
  vector<vector<int> > order;
  order=cij;

  for (int i = 0; i < cij.size(); ++i)
    {
      for (int j = 0; j < cij[0].size(); ++j)
      {
       order[i][j]=0;
      }
    }

  for (int i = 0+resolution_of_digestion + nearest_neighbour_scope  ; i < cij.size()-resolution_of_digestion - nearest_neighbour_scope  ; ++i)
  {
    for (int j = 0+resolution_of_digestion + nearest_neighbour_scope  ; j < cij[0].size()-resolution_of_digestion - nearest_neighbour_scope  ; ++j)
    { 



             ///consider only the major nodes
      if(cij[i][j]==2)
      {   ////look for major nodes in the neighbourhood
        for (int x = -1*resolution_of_digestion - nearest_neighbour_scope  ; x < resolution_of_digestion+1 + nearest_neighbour_scope  ; x++)
        {  
          for (int y = -1*resolution_of_digestion - nearest_neighbour_scope  ; y < resolution_of_digestion+1 + nearest_neighbour_scope  ; ++y)
          {   ///conditional to locate a major node neighbour 
            if(cij[i+x][j+y]==2)
            {   Point p1,p2;
              p1.x=i;
              p1.y=j;
              p2.x=i+(x);
              p2.y=j+y;
              float filled_ratio=0;
              int count_of_minor_nodes=0;
              LineIterator it(image, p1, p2, 8);


              ///loop to traverse the line joning the node and its neighbour
              for(int ii = 0; ii < it.count; ii++, ++it)
              {   ///loop to search the neighbourhood of each of the points of the line 

                for (int xx = -1*line_traversal_scope; xx < line_traversal_scope+1; xx++)
                {  
                  for (int yy = -1*line_traversal_scope; yy < line_traversal_scope+1; ++yy)
                  ///check whether the point of the line is a minor passing node
                  {
                    Point line_point =   it.pos();
                  
                      int search_point_x = line_point.x+xx;
                      int search_point_y = line_point.y+yy;
          
                        if(cij[search_point_x][search_point_y]==1)
                        {
                          count_of_minor_nodes++;
                        }
                  }
                }
                filled_ratio=(count_of_minor_nodes)/(it.count);
                RNG& rng = theRNG();
                if(filled_ratio>=Tolerance)
                {  if(p1==p2) break;
                  line(image, p1, p2, CV_RGB(rng.uniform(254,255),rng.uniform(254,255),rng.uniform(254,255)),rng.uniform(radius_of_major_nodes,radius_of_major_nodes), 8);
                  count_of_minor_nodes=0;
                  order[p1.x][p1.y]++;
                  order[p2.x][p2.y]++;
                }
              }
              
            }
          }
        }
      }
    }
  }
  

  for (int i = 0; i < cij.size(); ++i)
    {
      for (int j = 0; j < cij[0].size(); ++j)
      { Point pt(i,j);
       switch(order[i][j])
       {
        case 0: break;
        case 1:  circle(image, cvPoint(i, j),radius_of_major_nodes, CV_RGB(255,255, 0),-1); break;
        case 2: circle(image, cvPoint(i, j),radius_of_major_nodes, CV_RGB(0, 255, 0),-1);  break;
        case 3: circle(image, cvPoint(i, j),radius_of_major_nodes, CV_RGB(0, 0, 255),-1);  break;
        default : circle(image, cvPoint(i, j),radius_of_major_nodes, CV_RGB(250, 125, 255),-1); break ;
       }
      }
    }

}

void init_flow()
{
  namedWindow("Flow", CV_WINDOW_AUTOSIZE); //create a window called "Control"
  createTrackbar("field_generate", "Flow", &field_generate, 1);
    createTrackbar("countour_field_switch", "Flow", &countour_field_switch, 1);
  
  createTrackbar("region_of_interest_switch", "Flow", &region_of_interest_switch, 1);
  createTrackbar("canny", "Flow", &canny_switch, 1);
    createTrackbar("calc_cij_switch", "Flow", &calc_cij_switch, 1);

  createTrackbar("node_draw_switch", "Flow", &node_draw_switch, 1);
   namedWindow("Canny", CV_WINDOW_AUTOSIZE);
   createTrackbar("canny_thresh", "Canny", &canny_thresh, 500);
   createTrackbar("canny_ratio", "Canny", &canny_ratio, 5);
   createTrackbar("canny_kernel", "Canny", &canny_kernel, 10);
}




void init_grass_interface()
{
  namedWindow("Control", CV_WINDOW_AUTOSIZE); //create a window called "Control"

  

  //Create trackbars in "Control" window
  createTrackbar("LowH", "Control", &iLowH, 179); //Hue (0 - 179)
  createTrackbar("HighH", "Control", &iHighH, 179);

  createTrackbar("LowS", "Control", &iLowS, 255); //Saturation (0 - 255)
  createTrackbar("HighS", "Control", &iHighS, 255);

  createTrackbar("LowV", "Control", &iLowV, 255);//Value (0 - 255)
  createTrackbar("HighV", "Control", &iHighV, 255);

  createTrackbar("smooth", "Control", &smooth, 1);
  createTrackbar("smooth_size", "Control", &smooth_size, 5);
  createTrackbar("blur_median", "Control", &blur_median, 10);
}

void init_line_interface()
{
  namedWindow("Control_line", CV_WINDOW_AUTOSIZE); //create a window called "Control"

  

  //Create trackbars in "Control" window
  createTrackbar("LowH", "Control_line", &iLowH_line, 179); //Hue (0 - 179)
  createTrackbar("HighH", "Control_line", &iHighH_line, 179);

  createTrackbar("LowS", "Control_line", &iLowS_line, 255); //Saturation (0 - 255)
  createTrackbar("HighS", "Control_line", &iHighS_line, 255);

  createTrackbar("LowV", "Control_line", &iLowV_line, 255);//Value (0 - 255)
  createTrackbar("HighV", "Control_line", &iHighV_line, 255);

  createTrackbar("smooth", "Control_line", &smooth_line, 1);
  createTrackbar("smooth_size", "Control_line", &smooth_size_line, 5);

  createTrackbar("Add_line", "Control_line", &Add_line, 5);
  createTrackbar("line_cij", "Control_line", &line_cij, 5);
 // createTrackbar("smooth_size", "Control_line", &smooth_size_line, 5);

}
void init_node_control()
{
  namedWindow("Node_Control", CV_WINDOW_AUTOSIZE);
  createTrackbar( "amount_of_white_neighbours", "Node_Control", &amount_of_white_neighbours,9);
  //createTrackbar( "circle", "Node_Control", &b,255);
  createTrackbar( "digesting reso", "Node_Control", &resolution_of_digestion,255);
  createTrackbar( "nearest_neighbour_scope", "Node_Control", &nearest_neighbour_scope,255);
  createTrackbar( "Edge Tolerance", "Node_Control", &Tolerance,40);
  createTrackbar( "line_traversal_scope", "Node_Control", &line_traversal_scope,2);
  createTrackbar( "radius_of_major_nodes", "Node_Control", &radius_of_major_nodes,25);
}


void contourify(Mat &imgHull,bool max=true)
{
  vector<vector<Point> > contours;
  vector<Vec4i> hierarchy;
  findContours( imgHull, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
  vector<vector<Point> > hull( contours.size() );
  for( int i = 0; i < contours.size(); i++ )
  {
    convexHull( Mat(contours[i]), hull[i], false );
  }
  Mat drawing = Mat::zeros( imgHull.size(), CV_8UC3 );
  for( int i = 0; i< contours.size(); i++ )
  {
    Scalar color =Scalar(255,255,255);
    drawContours( drawing, contours, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
    drawContours( drawing, hull, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
  }
  if(max)
  {
    int max_area_i=0;
    for( int i = 0; i< contours.size(); i++ )
    {
      if(contourArea(contours[i])>contourArea(contours[max_area_i]))
      {
        max_area_i=i;
      }
    }
    drawContours( drawing, hull, max_area_i, Scalar(5,195,25), -1, 8, vector<Vec4i>(), 1, Point() );
    field=hull[max_area_i];
  }
  imgHull=drawing;
  
}


void HSV_threshold(Mat &imgOriginal,Mat  &imgThresholded)
{
  cvtColor(imgOriginal, imgOriginal, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV

  inRange(imgOriginal, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded); //Threshold the image

  if(smooth)
  {
    //morphological opening (removes small objects from the foreground)
    erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(smooth_size, smooth_size)));
    dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(smooth_size, smooth_size)) ); 

    //morphological closing (removes small holes from the foreground)
    dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(smooth_size, smooth_size)) ); 
    erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(smooth_size, smooth_size)) );
  }
  cvtColor(imgOriginal, imgOriginal, COLOR_HSV2BGR);
  medianBlur(imgThresholded,imgThresholded,(blur_median*2+1));
}

void HSV_threshold_line(Mat &imgOriginal,Mat  &imgThresholded)
{
  cvtColor(imgOriginal, imgOriginal, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV

  inRange(imgOriginal, Scalar(iLowH_line, iLowS_line, iLowV_line), Scalar(iHighH_line, iHighS_line, iHighV_line), imgThresholded); //Threshold the image

  if(smooth_line)
  {
    //morphological opening (removes small objects from the foreground)
    erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(smooth_size_line, smooth_size_line)));
    dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(smooth_size_line, smooth_size_line)) ); 

    //morphological closing (removes small holes from the foreground)
    dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(smooth_size_line, smooth_size_line)) ); 
    erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(smooth_size_line, smooth_size_line)) );
  }
  cvtColor(imgOriginal, imgOriginal, COLOR_HSV2BGR);
}
void infield(Mat &img)
{
  Size size=img.size();
  for(int j=0;j<size.height;j++)
  {
    for(int i=0;i<size.width;i++)
    { Point pt(i,j);

      if(pointPolygonTest(field,pt,false)!=1)
      {
        img.at<Vec3b>(j,i)[0] = 0; 
        img.at<Vec3b>(j,i)[1] = 0; 
        img.at<Vec3b>(j,i)[2] = 0; 
      }
    }
  }
}
void canny(Mat &src_gray)
{
  Mat canny_output;
  vector<vector<Point> > contours;
  vector<Vec4i> hierarchy;

  /// Detect edges using canny
  Canny( src_gray, canny_output, canny_thresh, canny_thresh*canny_ratio, canny_kernel*2+1);
  /// Find contours
  findContours( canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

  /// Draw contours
  Mat drawing = Mat::zeros( canny_output.size(), CV_8UC3 );
  for( int i = 0; i< contours.size(); i++ )
     {
       Scalar color = Scalar(5,255,151);
       drawContours( drawing, contours, i, color, -2, 8, hierarchy, 0, Point() );
     }

  /// Show in a window
  src_gray=drawing;
}
void thin(Mat &img)
{
  img.convertTo(img,CV_8UC1);
  cv::Mat skel(img.size(), CV_8UC1, cv::Scalar(0));
  cv::Mat temp(img.size(), CV_8UC1);
  cv::Mat element = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3));
  bool done;
  do
  {
    cv::morphologyEx(img, temp, cv::MORPH_OPEN, element);
    cv::bitwise_not(temp, temp);
    cv::bitwise_and(img, temp, temp);
    cv::bitwise_or(skel, temp, skel);
    cv::erode(img, img, element);
   
    double max;
    cv::minMaxLoc(img, 0, &max);
    done = (max == 0);
  } while (!done);
  img=skel;
}
int main( int argc, char** argv )
{
  if(argc<2)
  {
    printf("./object filename \n");
   return 0;
  }

  init_grass_interface();
  init_line_interface();
  init_flow();
  init_node_control();

    

  while (true)
  {
    Mat imgOriginal;
    Mat imgThresholded;
    Mat imgThresholded_line;
    Mat imgHull;
    imgOriginal=imread(argv[1]);
    Mat imgField=imread(argv[1]);
    Mat imgField2;

    int IMAGE_WIDTH = imgOriginal.cols;
     int IMAGE_HEIGHT = imgOriginal.rows;


    vector<vector<int> > cij(vector<vector<int> >(IMAGE_WIDTH, vector<int>(IMAGE_HEIGHT, INF)));


    HSV_threshold(imgOriginal,imgThresholded);  
    
    imgHull=imgThresholded.clone(); 
    if(field_generate)
    {    
      contourify(imgHull);
    }

    

    if(region_of_interest_switch&&field_generate)
    {   
      infield(imgField);
    }


    imgField2=imgField.clone();


    HSV_threshold_line(imgField2,imgThresholded_line);
     thin(imgThresholded_line);


     if(region_of_interest_switch&&field_generate)
    {
      if(canny_switch)
      {
        canny(imgField);
        if(countour_field_switch)
        {
              //morphological opening (removes small objects from the foreground)
        erode(imgField, imgField, getStructuringElement(MORPH_ELLIPSE, Size(smooth_size, smooth_size)));
        dilate( imgField, imgField, getStructuringElement(MORPH_ELLIPSE, Size(smooth_size, smooth_size)) ); 

        //morphological closing (removes small holes from the foreground)
     //   dilate( imgField, imgField, getStructuringElement(MORPH_ELLIPSE, Size(smooth_size, smooth_size)) ); 
      //  erode(imgField, imgField, getStructuringElement(MORPH_ELLIPSE, Size(smooth_size, smooth_size)) );
        }

        canny(imgThresholded_line);
        if(calc_cij_switch)
        {
          calcCij(imgField, cij);

          //printf("yolo yolo");

          if(node_draw_switch)
          {
            createImageNodes(imgField, cij);
            createImageEdges(imgField,cij);
           // printf("nigga nigga");
          }
        }
      }
    }

    if(countour_field_switch&&!canny_switch)
    {
      contourify(imgThresholded_line,false);
    }
     
    imgOriginal.convertTo(imgOriginal,CV_8UC3);
    imgField.convertTo(imgField,CV_8UC3);
    imgHull.convertTo(imgHull,CV_8UC3);
    imgOriginal+=imgField;
    if(line_cij)
      {
       
      //  imgThresholded_line.convertTo(imgThresholded_line,CV_8UC3);
        calcCij(imgThresholded_line,cij);
        createImageNodes(imgThresholded_line,cij);
        createImageEdges(imgThresholded_line,cij);
      }
    if(Add_line)
    {
      imgThresholded_line.convertTo(imgThresholded_line,CV_8UC3);
      imgOriginal+=imgThresholded_line;
    }


  //  imgOriginal+=imgThresholded_line;
    imshow("Original", imgOriginal); //show the original image
    imshow("Thresholded Image", imgThresholded);
    imshow("Thresholded Image_line", imgThresholded_line); //show the thresholded image   
    imshow("convexHull",imgHull);
    imshow("region_of_interest",imgField);
    

    waitKey(1); 
  }

  return 0;
}
