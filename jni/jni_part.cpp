#include <jni.h>
#include <vector>

#include <stdio.h>
#include <algorithm>
#include <iostream>
#include <ctime>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <map>
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/ml/ml.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/video/tracking.hpp"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <fstream>
#include <android/log.h>
#include <jni.h>

#define SDCARD_PATH "/sdcard/"
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))


using namespace std;
using namespace cv;


TermCriteria termcrit(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03);
Size winSize(10,10);

Point2f pt;
string root;   // the path to the sdcard

const int MAX_COUNT = 500;
bool needToInit = false;
Mat img_scene, img_prevscene;
Mat img_overlay, cur_overlay;
vector<Point2f> points[2];
int flag = 0;
Mat dbImg, dbDes;
vector<KeyPoint> dbKeys;

/** create a mask for the source img_objects to highlight the obj regions **/
Mat source_mask;

int nObj, nSteps;
map< int, map<int, string> > Steps;
int currStep;
vector<Rect> objLocs, objLocsScene;
double x1, x2, y_1, y2;
vector<Point2f> source_points;
vector<Point2f> scene_points;
vector<Point2f> prev_scene_points;

/** detect SIFT keypoints in the mask regions of source img_objects and in the scene image**/
FastFeatureDetector detector;
SiftFeatureDetector detector1;
vector<KeyPoint> keypoints_source;
vector<KeyPoint> keypoints_scene;

/** compute SIFT descriptors in the source img_objects and in the scene image**/
SiftDescriptorExtractor extractor;
Mat descriptors_source, descriptors_scene;

vector< vector<Point> > ObjectContours;
float currentPose[16];

int temp_flag = 0;
Mat rem_H;

void mergeImgs(Mat& orig, Mat snap) {
	for (int i = 0; i < snap.rows; i++) {
		for (int j = 0; j < snap.cols; j++) {
			if (norm(snap.at<Vec4b>(i,j)) > 10) {
				orig.at<Vec4b>(i,j) = snap.at<Vec4b>(i,j);
			}
		}
	}
}

void  HomographicTransformation( vector<Point2f>& obj, vector<Point2f>& scene ){
	vector<uchar> inliers;
		rem_H = findHomography( obj, scene, CV_RANSAC, 5, inliers );

		warpPerspective(img_overlay, cur_overlay, rem_H, img_scene.size());
		cur_overlay.copyTo(img_overlay);
		LOGI("img_overlay alpha channels %d", img_overlay.channels());
//		imwrite("/sdcard/charminarAR/curOverlay.jpg", cur_overlay);

//	H = Mat::eye(3, 3, 	CV_64F);
	LOGI("Computed Homography");

	char temp[100];
	sprintf(temp, "size: %d %d", cur_overlay.rows, cur_overlay.cols);
	LOGI(temp);

	LOGI("tx the overlay");
	//
	//
	// 	  int count = 0;
	// 	  for( int i = 0 ; i < obj.size() ; i++ ){
	// 		  if( !inliers[i] ){
	// 			  obj.erase( obj.begin() + i );
	// 			  scene.erase( scene.begin() + i );
	// 		  }else{
	// 			  count++;
	// 		  }
	// 	  }
	// 	  if( count > 20 ){
	// 		  perspectiveTransform( prev_scene_points, scene_points, H );
	// 		  flag = 1;
	// 		  LOGI("Perspective Transformation done");
	//
	// 	  }else {
	// 		  flag = 0;
	// 		  LOGI("Not enough inliers, objects not in scene");
	// 	  }
}

vector< Point2f > getKeys( Mat& img_scene ){
	detector.detect( img_scene, keypoints_scene );
	LOGI("Keypoints detected");

	vector<Point2f> currFrameKeys;
	for( int i = 0 ; i < keypoints_scene.size() ; i++ ){
		currFrameKeys.push_back(keypoints_scene[i].pt);
	}
	return currFrameKeys;
}

void Detect( Mat& img_scene ){
	LOGI("starting object detection");
	detector1.detect( img_scene, keypoints_scene );
	LOGI("Keypoints detected");

	extractor.compute( img_scene, keypoints_scene, descriptors_scene );
	LOGI("Descriptors extracted");

	FlannBasedMatcher matcher;
	std::vector< DMatch > matches;
	matcher.match( descriptors_source, descriptors_scene, matches );
	LOGI("Matching done");

	//-- Quick calculation of max and min distances between keypoints
	double min_dist=1000, max_dist;
	for( int i = 0; i < descriptors_source.rows; i++ )
	{ double dist = matches[i].distance;
	if( dist < min_dist ) min_dist = dist;
	if( dist > max_dist ) max_dist = dist;
	}

	//-- Draw only "good" matches (i.e. whose distance is less than 3*min_dist )
	std::vector< DMatch > good_matches;

	for( int i = 0; i < descriptors_source.rows; i++ )
	{ if( matches[i].distance <= 4*min_dist )
	{ good_matches.push_back( matches[i]); }
	}

	// GEOM FILTER

	good_matches.clear();
	vector<uchar> inliers;
	vector<Point2f> pts1, pts2;
	for (int i = 0; i < matches.size(); i++) {
		pts1.push_back(keypoints_source[matches[i].queryIdx].pt);
		pts2.push_back(keypoints_scene[matches[i].trainIdx].pt);
	}
	Mat F = findFundamentalMat(Mat(pts1), Mat(pts2),
			FM_RANSAC, 3, 0.99, inliers);
	for (int i = 0; i < inliers.size(); i++) {
		if ( (int)inliers[i] ) {
			good_matches.push_back(matches[i]);
		}
	}

	//-- Localize the object
	std::vector<Point2f> obj;
	std::vector<Point2f> scene;

	for( int i = 0; i < good_matches.size(); i++ )
	{
		//-- Get the keypoints from the good matches
		obj.push_back( keypoints_source[ good_matches[i].queryIdx ].pt );
		scene.push_back( keypoints_scene[ good_matches[i].trainIdx ].pt );
	}

	LOGI("Point Correspondence done");

	Mat img_matches;
	Mat img_object = imread("/sdcard/charminarAR/obj.jpg");
	drawMatches( img_object, keypoints_source, img_scene, keypoints_scene,
			good_matches, img_matches, Scalar::all(-1), Scalar::all(-1),
			vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );
	imwrite("/sdcard/charminarAR/matches2.jpg", img_matches);
	LOGI("saved matches");

	prev_scene_points = source_points;
	HomographicTransformation(  obj, scene );
	points[1] = scene;
}


extern "C" {
JNIEXPORT jint JNICALL Java_com_example_enginear_Sample3View_LoadSource(JNIEnv* env, jobject thiz, jint flagS )
{
	root = string(SDCARD_PATH);
	FileStorage fs(root + "charminarAR/obj-kpts-desc.yml", FileStorage::READ);
	if (!fs.isOpened()) {
		LOGI("Unable to open the object keypoints file!!!");
		return -1;
	}
	FileNode kpts = fs["kpts"];
	read(kpts, keypoints_source);
	FileNode desc = fs["desc"];
	read(desc, descriptors_source);
	fs.release();

	img_overlay = imread(root + "charminarAR/overlay.png", -1);

}
}

extern "C" {
JNIEXPORT jint JNICALL Java_com_example_enginear_Sample3View_FindFeatures(JNIEnv* env, jobject thiz, jint width, jint height, jbyteArray yuv, jintArray bgra, jintArray bgra_overlay, jboolean addRemovePt, jfloat touchX, jfloat touchY, jint cStep )
{
	jbyte* _yuv  = env->GetByteArrayElements(yuv, 0);
	jint*  _bgra = env->GetIntArrayElements(bgra, 0);
	jint*  _bgra_overlay = env->GetIntArrayElements(bgra_overlay, 0);


	Mat myuv(height + height/2, width, CV_8UC1, (unsigned char *)_yuv);
	Mat mbgra(height, width, CV_8UC4, (unsigned char *)_bgra);
	Mat mbgra_overlay(height, width, CV_8UC4, (unsigned char *)_bgra_overlay);
	Mat mgray(height, width, CV_8UC1, (unsigned char *)_yuv);

	currStep = cStep;


	LOGI("BEFORE");
	mgray.copyTo(img_scene);
//	img_scene = img_scene(Rect(0,0,1024,720));
	LOGI("AFTER");

//	double aspect_ratio = img_scene.rows*1.0/img_scene.cols;
//	resize(img_scene, img_scene, Size(1024, ));

	char temp[100];
	sprintf(temp, "img-scene size: %d %d", img_scene.rows, img_scene.cols);
	LOGI(temp);


	//Please make attention about BGRA byte order
	//ARGB stored in java as int array becomes BGRA at native level
	Mat img_scene2;
	cvtColor(myuv, mbgra, CV_YUV420sp2BGR, 4);
//	cvtColor(myuv, mbgra_overlay, CV_YUV420sp2BGR, 4);

	cvtColor(myuv, img_scene2, CV_YUV420sp2BGR, 3);
	//    LOGI("Check rgb: %d %d %s", img_scene2.cols, img_scene2.rows, img_scene.channels() );


	vector<KeyPoint> qKeys;
	Mat qDes;
	int pFlag =0;
	if( addRemovePt )
	{
		points[0].clear();
		points[1].clear();
		Detect( img_scene );
		//mergeImgs(mbgra, cur_overlay);
		LOGI("Detected");
//		add( mbgra, 1.0f, cur_overlay, 1.0f, 0.0, mbgra);

		char temp[100];
		sprintf(temp, "size: %d %d", cur_overlay.rows, cur_overlay.cols);
		LOGI(temp);

		cvtColor(cur_overlay,mbgra_overlay,CV_BGR2BGRA);
		imwrite("/sdcard/charminarAR/mbgra.jpg", mbgra);
//		Mat temp_mbgra = mbgra.clone();
//		mergeImgs(mbgra, cur_overlay);

		LOGI("added with overlay");
//		imwrite("/sdcard/charminarAR/curOverlay2.png", cur_overlay);
		addRemovePt = false;
		pFlag = 10;
	}
	else if( !points[0].empty() )
	{
		LOGI("points[0] not empty!");
		vector<uchar> status;
		vector<float> err;
		if(img_prevscene.empty())
			img_scene.copyTo(img_prevscene);
		calcOpticalFlowPyrLK(img_prevscene, img_scene, points[0], points[1], status, err, winSize,
				3, termcrit, 0);
		LOGI("Computing optical flow");

		size_t i, k;
		for( i = k = 0 ; i < points[1].size() ; i++ ){

			if( !status[i] ){
				points[1].erase( points[1].begin() + i);
				points[0].erase( points[0].begin() + i);
				continue;
			}
		}
		if( points[1].size() > 15 ){
			HomographicTransformation( points[0], points[1] );

			cvtColor(cur_overlay,mbgra_overlay,CV_BGR2BGRA);
//			mergeImgs(mbgra, cur_overlay);
		}else{
			Detect( img_scene );
		}
		if ( flag == 0 ){
			return -1;
		}

		points[1].clear();
		points[1] = getKeys(img_scene);


	}else{
		pFlag = -1;
	}

	if( pFlag != -1 ){
	   vector<int> compression_params;
	    compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
	    compression_params.push_back(9);
		imwrite("/sdcard/charminarAR/overlay3_1.png", mbgra_overlay, compression_params);
	}
	std::swap(points[1], points[0]);
	swap(img_prevscene, img_scene);
	prev_scene_points = scene_points;


	env->ReleaseIntArrayElements(bgra, _bgra, 0);
	env->ReleaseIntArrayElements(bgra_overlay, _bgra_overlay, 0);
	env->ReleaseByteArrayElements(yuv, _yuv, 0);
	return pFlag;
}

}

