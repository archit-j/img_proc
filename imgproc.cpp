#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;
using namespace std;

//for the field green
int iLowH = 170;
int iHighH = 179;

int iLowS = 150; 
int iHighS = 255;

int iLowV = 60;
int iHighV = 255;

int smooth=1;

void init_interface()
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
}

void HSV_threshold(Mat &imgOriginal,Mat  &imgThresholded)
{
  cvtColor(imgOriginal, imgOriginal, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV

  inRange(imgOriginal, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded); //Threshold the image

  if(smooth)
  {
  //morphological opening (removes small objects from the foreground)
  erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
  dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) ); 

  //morphological closing (removes small holes from the foreground)
  dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) ); 
  erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
  }
  cvtColor(imgOriginal, imgOriginal, COLOR_HSV2BGR);
}
int main( int argc, char** argv )
{
  if(argc<2)
  {
    printf("./object filename \n");
   return 0;
  }

  init_interface();
  Mat imgTmp;
  imgTmp=imread(argv[1]);

  while (true)
  {
    Mat imgOriginal;

    imgOriginal=imread(argv[1]);

    Mat imgHSV;

    
   
    Mat imgThresholded;
    HSV_threshold(imgOriginal,imgThresholded);       
    imshow("Original", imgOriginal); //show the original image
    imshow("Thresholded Image", imgThresholded); //show the thresholded image   
    waitKey(1); 
  }

  return 0;
}
