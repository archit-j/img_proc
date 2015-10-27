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

int iLowH_line = 25;
int iHighH_line = 51;

int iLowS_line = 135; 
int iHighS_line = 255;

int iLowV_line = 12;
int iHighV_line = 106;


int smooth=1;
int smooth_size=5;

int smooth_line=1;
int smooth_size_line=5;

int countour_field_switch=0;
int field_generate=0;
int region_of_interest_switch=0;
int canny_switch=0;
int canny_thresh=50;
int blur_median=3;


int amount_of_white_neighbours=2;
int resolution_of_digestion=12;
int radius_of_major_nodes=5;

void calcCij(Mat &image, vector<vector<int> > &cij)
{
  int IMAGE_WIDTH = image.cols;
  int IMAGE_HEIGHT = image.rows;
  cij.clear();
  cij.resize(IMAGE_WIDTH, vector<int>(IMAGE_HEIGHT, INF));
  vector<vector<int> > whiteCount(vector<vector<int> >(IMAGE_WIDTH, vector<int>(IMAGE_HEIGHT, 0)));
  vector<vector<int> > digest(vector<vector<int> >(IMAGE_WIDTH, vector<int>(IMAGE_HEIGHT, 0)));
  vector<vector<int> > Order(vector<vector<int> >(IMAGE_WIDTH, vector<int>(IMAGE_HEIGHT, 0)));
  vector<vector<int> > parent(vector<vector<int> >(IMAGE_WIDTH, vector<int>(IMAGE_HEIGHT, 0)));
  for (int i = resolution_of_whitecount; i < IMAGE_WIDTH - resolution_of_whitecount; ++i)
  {
    for (int j = resolution_of_whitecount; j < IMAGE_HEIGHT-resolution_of_whitecount; ++j)
    {
      for (int x = -1*resolution_of_whitecount; x < resolution_of_whitecount+1; ++x)
      {
        for (int y = -1*resolution_of_whitecount; y < resolution_of_whitecount+1; ++y)
        {  Scalar intensity = image.at<uchar>(y+j, x+i);
          if(intensity.val[0]!=0)
          whiteCount[i][j]++;
        }
      }
      digest[i][j]=0;
      if(whiteCount[i][j]==amount_of_white_neighbours+1)
      {
        digest[i][j]=1;
        parent[i][j]=1;
        cij[i][j]=1;
      }
      if(whiteCount[i][j]==amount_of_white_neighbours+2)
      {
        digest[i][j]=3;
        parent[i][j]=1;
        cij[i][j]=1;
      }
      if(whiteCount[i][j]==amount_of_white_neighbours+3)
      {
        digest[i][j]=9;
        parent[i][j]=1;
        cij[i][j]=1;
      }
    }
  }
  for (int i = resolution_of_digestion; i < IMAGE_WIDTH - resolution_of_digestion; ++i)
  {
    for (int j = resolution_of_digestion; j < IMAGE_HEIGHT - resolution_of_digestion; ++j)
    {
      if(whiteCount[i][j] == 0)
        continue;
      int max_digest=0,maxi=0,maxj=0;
      if(digest[i][j]>=1)
      { max_digest=digest[i][j];
        maxj=j;
        maxi=i;
         int digested_count = 0;
        for (int x = -1*resolution_of_digestion; x < resolution_of_digestion+1; ++x)
        {   
          for (int y = -1*resolution_of_digestion; y < resolution_of_digestion+1; ++y)
          {  
            if(digest[i+x][j+y]>max_digest)
            { 
              max_digest=digest[i+x][j+y];
              maxi=i+x;
              maxj=j+y;
            }
            else 
            {
              digested_count+=digest[i+x][j+y];
              digest[i+x][j+y]=0;
            }
          }
        }
        digest[maxi][maxj]+=digested_count;
        cij[maxi][maxj]=2;
      }
    }
  }
}

void createImageNodes(Mat &image, vector<vector<int> > &cij)
{
  for (int i = 0; i < cij.size(); ++i)
  {
    for (int j = 0; j < cij[0].size(); ++j)
    { 
      
      { j/=3;
        if(cij[i][j] == 1)
        circle(image, Point(i, j), 3, CV_RGB(255, 0, 0));
        if(cij[i][j] ==2)
        circle(image, Point(i, j), radius_of_major_nodes, CV_RGB(0, 255, 0));
      }
    }
  }
}

void init_flow()
{
  namedWindow("Flow", CV_WINDOW_AUTOSIZE); //create a window called "Control"
  createTrackbar("field_generate", "Flow", &field_generate, 1);
    createTrackbar("countour_field_switch", "Flow", &countour_field_switch, 1);
  createTrackbar("canny_thresh", "Flow", &canny_thresh, 255);
  createTrackbar("region_of_interest_switch", "Flow", &region_of_interest_switch, 1);
  createTrackbar("canny", "Flow", &canny_switch, 1);
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

}
void init_node_control()
{
  namedWindow("Node_Control", CV_WINDOW_AUTOSIZE);
  createTrackbar( "amount_of_white_neighbours", "Node_Control", &amount_of_white_neighbours,9);
  //createTrackbar( "circle", "Node_Control", &b,255);
  createTrackbar( "digesting reso", "Node_Control", &resolution_of_digestion,255);
   //createTrackbar( "line", "Node_Control", &e,255, set_thresh );
  //createTrackbar( "Edge Tolerance", "Node_Control", &Tolerance,40);
  //createTrackbar( "line_traversal_scope", "Node_Control", &lts,2);
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
  Canny( src_gray, canny_output, canny_thresh, canny_thresh*2, 3 );
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
    vector<vector<int> > cij;

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
     if(region_of_interest_switch&&field_generate)
    {
      if(canny_switch)
      {
        canny(imgField);
        canny(imgThresholded_line);
        calcCij(imgField, cij);
        createImageNodes(imgField, cij);

      }

    }
    if(countour_field_switch)
    {
      contourify(imgThresholded_line,false);
    }
      imgThresholded_line.convertTo(imgThresholded_line,CV_8UC3);
    imgOriginal.convertTo(imgOriginal,CV_8UC3);
    imgField.convertTo(imgField,CV_8UC3);
    imgHull.convertTo(imgHull,CV_8UC3);
    imgOriginal+=imgField;


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
