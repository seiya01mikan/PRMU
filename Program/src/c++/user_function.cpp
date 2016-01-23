//
// サンプルコードとして，ブロックマッチングを用いたロゴの識別法を示します．
// 主な目的は，読み込んだアノテーション情報の取得方法や，画像の読み込み方，
// 認識結果の登録方法を示すことです．
//
// 情報を管理する配列には C++ 言語の std::list と std::vector が用いられており，
// こちらで用意した関数を用いずに，直接操作していただいても構いません．
//
// Here, we show a classification method by block matching as a sample code.
// The main purpose of us is to show how to access to the annotation information,
// read images, and append the classification results to the output lists.
//
// As arrays to manage the information, std::list and std::vector of C++ are used.
// One can use the member functions of them directly without using our prepared functions.
//


#include "prmu.hpp"

// include OpenCV libs
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include "opencv2/features2d/features2d.hpp"
#include <iostream>




// for the MSVC
#define CV_VERSION_STR CVAUX_STR(CV_MAJOR_VERSION) CVAUX_STR(CV_MINOR_VERSION) CVAUX_STR(CV_SUBMINOR_VERSION)
#ifdef _DEBUG
#define CV_EXT_STR "d.lib"
#else
#define CV_EXT_STR ".lib"
#endif

#pragma comment(lib, "opencv_core"	CV_VERSION_STR CV_EXT_STR)
#pragma comment(lib, "opencv_highgui"	CV_VERSION_STR CV_EXT_STR)
#pragma comment(lib, "opencv_imgproc"	CV_VERSION_STR CV_EXT_STR)


//
// ---------------------------------------------------------------------------------
//

using namespace std;
using namespace cv;

namespace sample{
  void dft_based_ncc( Mat& C, Mat& I, Mat& K );
  void psf2otf( Mat& dst, const Mat& src, size_t rows, size_t cols );
  void circshift( Mat& dst, const Mat& src, int dx, int dy );
}

//
// ---------------------------------------------------------------------------------
//

// int 型の値をstring型に変換する関数
inline std::string itos(int val)
{
  std::string ret;
  std::stringstream ss;
  ss << val;
  ss >> ret;
  return ret;
}


double thr = 0.0;

//
//
//

class Learn_
{
public:
  Mat img;
  Mat rota[4];
  Mat rota2[4];
  
  string Fname; // 画像ファイルの名前
  string Label; // 画像に対応するラベル
  string Fpath; // 画像ファイルへのパス
  
  Learn_(string name, string lab)
  {
    Fname = name;
    Label = lab;
  }
  
public:
  void setPath(string root)
  {
    Fpath = root + Fname;
    img = imread(Fpath);
  }

  string getLabel()
  {
    return Label;
  }
  
  string getPath()
  {
    return Fpath;
  }
  
  void setRota()
  {
	int angle = 90;
	int scale = 1;
	rota[0] = img.clone();

	flip(rota[0], rota2[0], 1); // 垂直軸で反転（水平反転）
	
	for(int i=1; i<4; i++){
	  
	  cv::Point2f center(img.cols*0.5, img.rows*0.5);
	  const cv::Mat affine_matrix = cv::getRotationMatrix2D( center, angle*i, scale );
	  warpAffine(img, rota[i], affine_matrix, img.size());
	  
	  flip(rota[i], rota2[i], 1); // 垂直軸で反転（水平反転）
	
	}
	
  }
  
};


//
//
//

#define MAX_DISTANCE 100000
#define NUM 16

typedef struct{
  int B;
  int G;
  int R;
  int s;
  int cx;
  int cy;
  Rect Box;
  int cla; //クラスタリングの判定
  Vec3b col;
  int dis;
  int diagonal;
  vector <cv::Point > region;
}component;

//vector<component> com;

typedef struct{
  int num;
  int dis;
}seldis;

int learnWhite[12];

int sumWhite(Mat img);
Mat color_reduce(Mat src);
bool judge_distance(vector<component> com, int i, int j);
void selectDistance(seldis* num, vector<component> com, int j);
vector<component> clustering2(vector<component> com, Mat src);
double overlap_Rate(Rect rec1, Rect rec2);
Mat reMser(Mat img);
vector<component> resultMSER(Mat gray, Mat output);
Mat thres(Mat roi, component com, Mat res);

vector<Learn_> store_learn(string root);

int matchTmp(vector<Learn_> le, Mat rect, Mat ms);
Mat Rota(Mat org, int ang);


//
//
//


void user_function(
		   //
		   // output
		   prmu::ImageList (&imlist_result)[3], // 結果情報の記録用
		   //
		   // input
		   size_t lv,
		   const prmu::ImageList& imlist_learn,
		   const prmu::ImageList (&imlist_test)[3]
		   )
{
  prmu::ImageList::const_iterator ite_learn, ite_test;
  prmu::ImageList::iterator ite_result;
  
  
  
  // 学習用画像の格納
  
  //vector<Learn_> learn_list = store_learn("../../img/myLearn/");
  vector<Learn_> learn_list = store_learn("");
  int number=1;

  //学習画像選択
  ite_learn = imlist_learn.begin();
  learn_list[0].img = imread(ite_learn->full_file_path());
  prmu::Rect roi1( ite_learn->rect_of_1st_img() );
  learn_list[0].img = learn_list[0].img( Rect(roi1.x, roi1.y, roi1.w, roi1.h) );

  for(int i = 0;i<21;i++){
    ++ite_learn;
  }

  learn_list[1].img = imread(ite_learn->full_file_path());
  prmu::Rect roi2( ite_learn->rect_of_1st_img() );
  learn_list[1].img = learn_list[1].img( Rect(roi2.x, roi2.y, roi2.w, roi2.h) );

  for(int i = 0;i<22;i++){
    ++ite_learn;
  }

  learn_list[2].img = imread(ite_learn->full_file_path());
  prmu::Rect roi3( ite_learn->rect_of_1st_img() );
  learn_list[2].img = learn_list[2].img( Rect(roi3.x, roi3.y+22, roi3.w-5, roi3.h-22) );

  for(int i = 0;i<16;i++){
    ++ite_learn;
  }

  learn_list[3].img = imread(ite_learn->full_file_path());
  prmu::Rect roi4( ite_learn->rect_of_1st_img() );
  learn_list[3].img = learn_list[3].img( Rect(roi4.x, roi4.y, roi4.w, roi4.h) );

  for(int i = 0;i<14;i++){
    ++ite_learn;
  }

  learn_list[4].img = imread(ite_learn->full_file_path());
  prmu::Rect roi5( ite_learn->rect_of_1st_img() );
  learn_list[4].img = learn_list[4].img( Rect(roi5.x, roi5.y, roi5.w, roi5.h) );
  
  for(int i = 0;i<17;i++){
    ++ite_learn;
  }

  learn_list[5].img = imread(ite_learn->full_file_path());
  prmu::Rect roi6( ite_learn->rect_of_1st_img() );
  learn_list[5].img = learn_list[5].img( Rect(roi6.x, roi6.y, roi6.w, roi6.h) );
  

  for(int i = 0;i<14;i++){
    ++ite_learn;
  }

  learn_list[6].img = imread(ite_learn->full_file_path());
  prmu::Rect roi7( ite_learn->rect_of_1st_img() );
  learn_list[6].img = learn_list[6].img( Rect(roi7.x, roi7.y, roi7.w, roi7.h) );

  for(int i = 0;i<7;i++){
    ++ite_learn;
  }

  learn_list[7].img = imread(ite_learn->full_file_path());
  prmu::Rect roi8( ite_learn->rect_of_1st_img() );
  learn_list[7].img = learn_list[7].img( Rect(roi8.x, roi8.y, roi8.w, roi8.h) );
  
  for(int i = 0;i<13;i++){
    ++ite_learn;
  }

  learn_list[8].img = imread(ite_learn->full_file_path());
  prmu::Rect roi9( ite_learn->rect_of_1st_img() );
  learn_list[8].img = learn_list[8].img( Rect(roi9.x, roi9.y, roi9.w, roi9.h) );
  
  for(int i = 0;i<13;i++){
    ++ite_learn;
  }

  learn_list[9].img = imread(ite_learn->full_file_path());
  learn_list[9].img = reMser(learn_list[9].img);
  prmu::Rect roi10( ite_learn->rect_of_1st_img() );
  learn_list[9].img = learn_list[9].img( Rect(roi10.x, roi10.y, roi10.w, roi10.h) );

  for(int i = 0;i<22;i++){
    ++ite_learn;
  }

  learn_list[10].img = imread(ite_learn->full_file_path());
  prmu::Rect roi11( ite_learn->rect_of_1st_img() );
  learn_list[10].img = learn_list[10].img( Rect(roi11.x, roi11.y, roi11.w, roi11.h) );
  
  for(int i = 0;i<15;i++){
    ++ite_learn;
  }

  learn_list[11].img = imread(ite_learn->full_file_path());
  prmu::Rect roi12( ite_learn->rect_of_1st_img() );
  learn_list[11].img = learn_list[11].img( Rect(roi12.x, roi12.y, roi12.w, roi12.h) );

  // 学習用画像の処理
  for(int i=0; i<learn_list.size(); i++){
    cvtColor(learn_list[i].img, learn_list[i].img, CV_BGR2GRAY);
    threshold(learn_list[i].img, learn_list[i].img, 0, 255, THRESH_BINARY|THRESH_OTSU);
	
    if(learn_list[i].Label == "LECOQ" || 
       learn_list[i].Label == "NB" ||
       learn_list[i].Label == "SHUN" ||
       learn_list[i].Label == "UA" 
       )
      learn_list[i].img =~ learn_list[i].img;
  
    morphologyEx(learn_list[i].img, learn_list[i].img, MORPH_CLOSE, cv::Mat(), cv::Point(-1,-1), 1);	 
	resize(learn_list[i].img, learn_list[i].img, cv::Size(100,100));
	
	learn_list[i].img =~ learn_list[i].img;
	learn_list[i].setRota();

	learnWhite[i]= sumWhite(learn_list[i].img);
	
  }
  
  
  // 課題画像の処理に入る
  for ( size_t _lv = 0; _lv < lv; ++_lv )
    {
      // アノテーション情報へのポインタ（イテレータ）
      // The pointer (actually iterator) of annotation information
      ite_test = imlist_test[_lv].begin();     // 入力用
      ite_result = imlist_result[_lv].begin(); // 結果用

      // アノテーション情報にアクセスするには，(*ite_test).XXX や ite_test->XXX が使用できます．
      // The access to the annotation information (i.e., member variables and functions)
      // is performed by (*ite_test).XXX or ite_test->XXX


      for ( ; ite_test != imlist_test[_lv].end(); ++ite_test, ++ite_result ) // each test image
	{

	  ite_learn = imlist_learn.begin();
			  
	  Mat input_img = imread(ite_test->full_file_path());
	
	  Mat src = imread(ite_test->full_file_path());

	  Mat out = src.clone(), out2 = src.clone();
	  Mat res = src.clone(), res2 = src.clone(), res3 = src.clone();

	  Mat gray_img;
  
	  src = color_reduce(src);

 
	  Mat bila;
	  bilateralFilter(src, bila, 11, 100, 64);
	  src = bila.clone();
	  cvtColor( src, gray_img, CV_BGR2GRAY );
  
	  Mat equalHist = gray_img.clone();
	  equalizeHist(gray_img, equalHist);
	  
	  vector<component> com = resultMSER(equalHist, out);

	  if(com.size() < 1){
	    component ex2;
	    ex2.B=0;
	    ex2.G=0;
	    ex2.R=0;
	    ex2.s=0;
	    ex2.cx=0;
	    ex2.cy=0;
	    ex2.Box=Rect(0,0,src.cols, src.rows);
	    ex2.cla=0; //クラスタリングの判定
	    ex2.dis=0;
	    ex2.col = Vec3b(0,0,0);
	    ex2.diagonal=0;
	    com.push_back(ex2);
	  }
	  
	  //色でクラスタリング
	  vector<component> c = clustering2(com, res2);
	  
	  string labname;
	  int rint;

	  for(int n=0; n<c.size(); n++){

	    if(c[n].Box.width > c[n].Box.height*3 || 
	       c[n].Box.height > c[n].Box.width*3 ||
	       ( c[n].Box.width < 40 && c[n].Box.height < 40) )continue;


	    /*MSER と　平均色による二値化*/
	    Mat roi_img(input_img, c[n].Box);
	    
	    Mat res = cv::Mat::zeros(gray_img.rows, gray_img.cols, CV_8UC3);
	    Mat res2;
	    Mat3b dotImg = res;
	    
	    for(unsigned int j=0; j<c[n].region.size(); j++){
	      dotImg(c[n].region.at(j)) = Vec3b(255,255,255);
	    }
	    Mat roi_img2(res, c[n].Box);
	    res2 = roi_img2.clone();//mser画像
	    

	    if(c[n].cla != 0){
	      res2 = thres(roi_img, c[n], res2);
	    }

	    rint = matchTmp(learn_list, roi_img, res2);
	  
	    int filaJ = 0;
	    if(rint != -1){
	      labname = learn_list[rint].Label;
	      
	      prmu::Rect bbox;
	      bbox.x = c[n].Box.tl().x; // 矩形の左上の座標 (x,y) 横幅と縦幅 (w,h)
	      bbox.y = c[n].Box.tl().y;
	      bbox.w = c[n].Box.width; 
	      bbox.h = c[n].Box.height;

	      if(rint == 2){
		filaJ = 1;
		bbox.w *= 1.1;
		bbox.y = bbox.y - (bbox.h * 0.55);
		bbox.h *= 1.59;
       
		if(bbox.y < 0)
		  bbox.y = 0;
		else if(bbox.x + bbox.w >= input_img.cols)
		  bbox.w = c[n].Box.width;
		else if(bbox.y + bbox.h >= input_img.rows)
		  bbox.h = c[n].Box.height;
		else{
		  filaJ = 0;
		}
	      }
	      

	      if(filaJ != 1)
		ite_result->append_result( prmu::label::str2label( labname ) , bbox );
	      
	      
	    }
	    
	    
	  }
	  		  

	} // end of the for-loop for test images
    } // end of the for-loop for levels

}

int sumWhite(Mat img){
  int sum = 0;
  for(int i=0; i<img.rows; i++){
    for(int j=0; j<img.cols; j++){
      if(img.at<unsigned char>(i,j) > 128)
	sum++;
    }
  }

  return sum;
}



vector<Learn_> store_learn(string root)
{
  vector<Learn_> llist;

  Learn_ l1("ASICS.jpg","ASICS");
  Learn_ l2("ASICS_a.jpg","ASICS");
  Learn_ l3("FILA.jpg","FILA");
  Learn_ l4("FILA_f.jpg","FILA");
  Learn_ l5("LECOQ.jpg","LECOQ");
  Learn_ l6("MIZUNO.jpg","MIZUNO");
  Learn_ l7("MIZUNO_m.jpg","MIZUNO");
  Learn_ l8("NB.jpg","NB");
  Learn_ l9("NB_n.jpg","NB");
  Learn_ l10("SHUN.jpg","SHUN");
  Learn_ l11("UA.jpg","UA");
  Learn_ l12("YONEX.jpg","YONEX");

  
  llist.push_back(l1);
  llist.push_back(l2);
  llist.push_back(l3);
  llist.push_back(l4);
  llist.push_back(l5);
  llist.push_back(l6);
  llist.push_back(l7);
  llist.push_back(l8);
  llist.push_back(l9);
  llist.push_back(l10);
  llist.push_back(l11);
  llist.push_back(l12);

  
  for(int i=0; i<12; i++)
    llist[i].setPath(root);

  
  return llist;
}




/*------------------------------------------------
  / 認識
  / -----------------------------------------------*/

//学習リスト，原画像切抜き，MSER画像切抜き
int matchTmp(vector<Learn_> le, Mat rect, Mat ms)
{
 
  
  double th[12]={
    0.5, // ASICS
    0.635, // ASICS_a
    0.76,  // FILA
    0.54, // FILA_f
    0.553,  // Lecoq
    0.545,  // MIZUNO
    0.5,  // MIZUNO_m
    0.4,  // NB
    0.62, // NB_n
    0.64, // SHUN
    0.7,  // UA
    0.53   // YONEX
  };
  
 
  int numb=-1;
  double MAXV=0;
 
  cvtColor(ms, ms, CV_BGR2GRAY);
  threshold(ms, ms, 0, 255, THRESH_BINARY|THRESH_OTSU);
  
  cv::dilate(ms, ms, Mat(), cv::Point(-1,-1), 3);
  cv::erode(ms, ms, Mat(), cv::Point(-1,-1), 3);

  resize(ms, ms, cv::Size(100,100));
 
  int msWhite = sumWhite(ms);

  
  for(int i=0; i<le.size(); i++){

    if(learnWhite[i]*2 > 10000)
      if(msWhite > 7500)
	continue;

    if(msWhite > learnWhite[i]*2 || msWhite < learnWhite[i]*0.7)
      continue;

	for(int j=0; j<4; j++){
	  
	  Mat tmp = le[i].rota[j].clone();		
	  
	  Mat A;
	  double maxv=0;
	  Point maxp;
	  
	  matchTemplate(ms, tmp, A, CV_TM_CCOEFF_NORMED);
	  minMaxLoc(A, NULL, &maxv, NULL, &maxp);
	  

	  if(MAXV < maxv){
		MAXV = maxv;
		numb = i;
	  }
    	  
	  // ここより水平反転の
	  Mat tmp2 = le[i].rota2[j].clone();
	
	  matchTemplate(ms, tmp2, A, CV_TM_CCOEFF_NORMED);
	  minMaxLoc(A, NULL, &maxv, NULL, &maxp);
	  

	  if(MAXV < maxv){
		MAXV = maxv;
		numb = i;
	  }

	}
  }

	
  if(MAXV >= th[numb]){
    
  }
  else
    return -1;
  
  return numb;
}

//
//
//



Mat Rota(Mat org, int ang)
{
  int xsize = org.cols;
  int ysize = org.rows;
  
  Mat ret( Size(xsize*2,ysize*2), org.type(), Scalar::all(255));
  
  for(int i=0; i<org.rows; i++){
    for(int j=0; j<org.cols; j++){
      ret.at<unsigned char>(i+ysize/2,j+xsize/2) = org.at<unsigned char>(i,j);
	  
    }
  }

  Point2f center(ret.cols*0.5, ret.rows*0.5);
  const cv::Mat affine_matrix = cv::getRotationMatrix2D( center, ang, 1);
  warpAffine(ret, ret, affine_matrix, ret.size(),1);

  for(int i=0; i<ret.rows; i++){
    for(int j=0; j<ret.cols; j++){
      if(ret.at<unsigned char>(i,j) == 0 || ret.at<unsigned char>(i,j) == 120)
	ret.at<unsigned char>(i,j) = 255;
	  
    }
  }


  return ret;
}


/*MSER*/

//減色処理
Mat color_reduce(Mat src){

  Mat img = cv::Mat::zeros(src.rows, src.cols, CV_8UC3);

  int cols = src.cols;
  int rows = src.rows;
  
  int reduce = 256 / NUM;
  for (int j = 0; j < rows; j++) {
    for (int i = 0; i < cols ; i++) {
      for (int k = 0; k < 3 ; k++) {
	img.at<cv::Vec3b>(j, i)[k] =  src.at<cv::Vec3b>(j, i)[k] / reduce * reduce + reduce/2;
      }
    }
  }
  return img;
}

//最短距離であるかの判断
bool judge_distance(vector<component> com, int i, int j){

  int match = 0;
  int max_dis = MAX_DISTANCE;
  for(int n=0; n<com.size(); n++){
    int count = 0;
    //com[n].cla = 0;
    int num = -1; //何番目のjとクラスタリングしたかの判定

    //中心座標からの距離
    int center_distance =
      (com[n].cx - com[j].cx)*(com[n].cx - com[j].cx) +
      (com[n].cy - com[j].cy)*(com[n].cy - com[j].cy);
    
    //BGRのユークリッド
    int BGR_distance = 
      (com[n].B - com[j].B)*(com[n].B - com[j].B) +
      (com[n].G - com[j].G)*(com[n].G - com[j].G) +
      (com[n].R - com[j].R)*(com[n].R - com[j].R);
    
    //各矩形の対角線の長さ
    int diagonal = 
      (com[n].Box.width * com[n].Box.width) +
      (com[n].Box.height * com[n].Box.height);
    
    if(BGR_distance < 400 && center_distance < diagonal){
      if(max_dis > center_distance){
	match = n;
	max_dis = center_distance;
      }
    }
  }

  if(match != i)
    return false;
  else
    return true;


}

//矩形間の距離が近い3つの距離を探索
void selectDistance(seldis* num, vector<component> com, int j){

  if(num[0].dis > com[j].dis){
    num[2].dis = num[1].dis;
    num[2].num = num[1].num;
    num[1].dis = num[0].dis;
    num[1].num = num[0].num;
    num[0].dis = com[j].dis;
    num[0].num = j;
  }
  else if(num[1].dis > com[j].dis){
    num[2].dis = num[1].dis;
    num[2].num = num[1].num;
    num[1].dis = com[j].dis;
    num[1].num = j;
  }
  else if(num[2].dis > com[j].dis){
    num[2].dis = com[j].dis;
    num[2].num = j;
  }
  
}


vector<component> clustering2(vector<component> com, Mat src){

  vector<component> com2(com);
  Rect rec(0,0,0,0);
  Mat dot = src.clone();

  for(int i=0; i<com.size(); i++){
    int count = 0;
    component tmp;
    tmp = com[i];
    int max_dis = MAX_DISTANCE;
    //com[i].cla = 0;
    Mat result = src.clone();
    int num = -1; //何番目のjとクラスタリングしたかの判定
    seldis selNum[3];
    for(int select=0; select<3; select++){
      selNum[select].num = -1;
      selNum[select].dis = 1000000;
    }
    
    rectangle(result, com[i].Box.tl(), com[i].Box.br(), Scalar(255,255,255), 2, CV_AA); 
    
    
    for(int j=0; j<com.size(); j++){
      if(i != j && com[j].cla != 1){
	//中心座標からの距離
	double center_distance =
	  (com[i].cx - com[j].cx)*(com[i].cx - com[j].cx) +
	  (com[i].cy - com[j].cy)*(com[i].cy - com[j].cy);
	
	//width , height を用いたdiagonalを用いる場合
	center_distance = sqrt(center_distance);


	//BGRのユークリッド
	double BGR_distance = 
	  (com[i].B/com[i].s - com[j].B/com[j].s)*(com[i].B/com[i].s - com[j].B/com[j].s) +
	  (com[i].G/com[i].s - com[j].G/com[j].s)*(com[i].G/com[i].s - com[j].G/com[j].s) +
	  (com[i].R/com[i].s - com[j].R/com[j].s)*(com[i].R/com[i].s - com[j].R/com[j].s);
	

	int diagonal = com[i].Box.width;
	if(com[i].Box.width > com[i].Box.height)
	  diagonal = com[i].Box.height;

	int diagonal2 = com[j].Box.width;
	if(com[j].Box.width > com[j].Box.height)
	  diagonal2 = com[j].Box.height;
	
	if(diagonal > diagonal2)
	  diagonal = diagonal2;
	
	Rect merge = com[i].Box & com[j].Box;	
	Rect merge2 = com[i].Box | com[j].Box;
	
	if(BGR_distance < 1000 && merge != com[i].Box && merge != com[j].Box){ 
	  com[j].dis = center_distance;
	  com[j].diagonal = diagonal;
	  selectDistance(selNum, com, j);
	  
	  
	}
	
      }
      
      
    }

    rec = com[i].Box;
    for(int sel=0; sel<3; sel++){
      if(selNum[sel].num != -1){
	
	if(com[selNum[sel].num].dis < com[selNum[sel].num].diagonal*3){
	  rec = rec | com[selNum[sel].num].Box;
	  tmp.B += com[selNum[sel].num].B;
	  tmp.G += com[selNum[sel].num].G;
	  tmp.R += com[selNum[sel].num].R;
	  tmp.s += com[selNum[sel].num].s;
	  copy(com[selNum[sel].num].region.begin(),com[selNum[sel].num].region.end(),std::back_inserter(tmp.region));
	}
	
      }
    }
  
    
    //同じ矩形を除去
    int cla = 0;
    for(int w=0; w<com2.size(); w++){
      Rect check = rec & com2[w].Box;
      Rect check2 = rec | com2[w].Box;
      if(check == check2)
	cla = 1;
      else{
	tmp.Box = rec;
      }
    }
    
    
    if(cla != 1){
      tmp.cla = 1;
      com2.push_back(tmp);
    }
    
  
  }
  
  
  return com2;
}


//重なり率を算出
double overlap_Rate(Rect rec1, Rect rec2){

  double s1 = rec1.width * rec1.height;
  double s2 = rec2.width * rec2.height;
  double res=0.0;

  if(s1 > s2)
    res = s2/s1;
  else
    res = s1/s2;
    
  return res;
}



//MSER 画像作成
Mat reMser(Mat img){

  Mat gray_img;
  cvtColor( img, gray_img, CV_BGR2GRAY );
  MSER mser(50, 60, 5000, 0.25, 0.2, 200, 1.01, 0.003, 5 );
  Mat result_img = cv::Mat::zeros(gray_img.rows, gray_img.cols, CV_8UC3);
  vector<KeyPoint> mser_features;
  vector<vector <cv::Point > > region;
  mser(gray_img, region);
  Mat3b dotImg = result_img;
  for(unsigned int i=0; i<region.size(); i++){
    for(unsigned int j=0; j<region.at(i).size(); j++){
      dotImg(region.at(i).at(j)) = Vec3b(255,255,255);
    }
  }

  return result_img;
}

vector<component> resultMSER(Mat gray, Mat output){
  vector<component> com;
  MSER mser(10, 50, 29000, 0.25, 0.2, 200, 1.01, 0.003, 5 );
  Mat src = output.clone();//Rectの大きさでクラスタリング  
  Mat result_img = cv::Mat::zeros(gray.rows, gray.cols, CV_8UC3);
  
  vector<KeyPoint> mser_features;
  vector<vector <cv::Point > > region;
  
  mser(gray, region);
  
  cv::Mat3b dotImg = result_img;
  Mat3b dotImg2= src;
  Rect box;
  //vector<Rect> rects;
  for(unsigned int i=0; i<region.size(); i++){
    component ex;
  
    
    double s = contourArea(region[i]);
    
    if(s < 10) continue;
    
    // 輪郭の点列を行列型に変換
    cv::Mat pointsf;
    cv::Mat(region[i]).convertTo(pointsf, CV_32F);
    // 輪郭を構成する点列をすべて包含する最小の矩形を計算
    box = cv::boundingRect(pointsf);
    
 

    //boxサイズで判定する
    if(box.size().height < box.size().width * 10 && box.size().width < box.size().height * 10){
    
      int B=0, G=0, R=0, ss=0;
      for(unsigned int j=0; j<region.at(i).size(); j++){
	Vec3b BGR = dotImg2(region.at(i).at(j));
	B += BGR[0];
	G += BGR[1];
	R += BGR[2];
	ss++;
      }

      //同じ矩形を除去
      
      int cla = 0;
      bool box_check = true;
      
      for(int w=0; w<com.size(); w++){
	Rect check = box & com[w].Box;
	Rect check2 = box | com[w].Box;
	if( (box.width > com[w].Box.width && box.height > com[w].Box.height) || 
	    (com[w].Box.width > box.width && com[w].Box.height > box.height) ){
	  double rate = overlap_Rate(check, check2);
	  if(rate > 0.87){
	    box_check = false;
	    com[w].B = B;
	    com[w].G = G;
	    com[w].R = R;
	    com[w].s = ss;
	    com[w].Box = box;
	    com[w].cx = box.tl().x + (box.br().x - box.tl().x)/2;
	    com[w].cy = box.tl().y + (box.br().y - box.tl().y)/2;
	    com[w].cla = 0;
	    com[w].region = region.at(i); 
	  }
	}
      }
      
      if(box_check){
	ex.B = B;
	ex.G = G;
	ex.R = R;
	ex.s = ss;
	ex.Box = box;
	ex.cx = box.tl().x + (box.br().x - box.tl().x)/2;
	ex.cy = box.tl().y + (box.br().y - box.tl().y)/2;
	ex.cla = 0;
	ex.region = region.at(i);
	com.push_back(ex);
      }
      
    }   
    else continue;

  }

  
  return com;
}

//平均色で二値化(RGBの値を用いたユークリッドで判定)
Mat thres(Mat roi, component com, Mat res){

 
  int B = com.B/com.s;
  int G = com.G/com.s;
  int R = com.R/com.s;

 for (int j = 0; j < roi.rows; j++) {
    for (int i = 0; i < roi.cols ; i++) {

      Vec3b bgr = roi.at<cv::Vec3b>(j,i);
      double BGR_distance = 
	(B - bgr[0])*(B - bgr[0]) + 
	(G - bgr[1])*(G - bgr[1]) + 
	(R - bgr[2])*(R - bgr[2]);

      if(BGR_distance < 3000){
	for (int k = 0; k < 3 ; k++) {
	  res.at<cv::Vec3b>(j, i) = Vec3b(255,255,255);
	}
      }

    }
  }
  


  return res;
}
