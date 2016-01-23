//
// アルゴリズムコンテスト2015
//
// お読みください
//
// 今回のアルゴリズムコンテストでは，学習用の画像とテスト用の画像が用意されており，
// テスト用の画像１枚を識別するために，複数枚の学習用の画像が必要となります．
// また，配布画像はJPEG形式であるため，windowsであれば OpenCV もしくは python の
// ライブラリを用いて読み込む必要があります．
// このような理由から，画像の読み込み方については参加者の方々にお任せしたいと思います．
//
// この main.cpp では，アノテーションファイル（annotation.txt）から
// 各画像に含まれるロゴのラベルや外接矩形情報を読み込み，user_function.cpp へと送る
// ことを目的とします．
//
// 参加者の方々には user_function.cpp を編集いただき，このファイル内にて，
// 画像の読み込みや，学習や識別といった処理を行っていただきます．
//
//
// Please read through it
//
// In this time algorithm contest (commonly, "Alcon2015"),
// images are prepared for the learning and the test process, respectively.
// To recognize a test-image, multiple images for learning will be needed.
// While, to deal with the JPEG file format of distributed images, some libraries
// such as OpenCV and python are required.
// From this reason, we shall leave the way of them to participants.
//
// This "main.cpp" file is designed to read label and bounding box information
// included in each image from "annotation.txt" files and send them to the
// "user_function.cpp".
//
// We would like participants to modify the "user_function.cpp" to implement
// image reading, learning, classification, and other operations.
//
//
// Operating environment
//
// The compilation is checked on the following environments:
// - Windows8, CPython 2.7 + OpenCV 2.4.10
// - OSX10.10, Default Python 2.7 + OpenCV 2.4.11
// - Ubuntu14.04, CPython 2.7 + OpenCV 2.4.8

#include "prmu.hpp"

//
// アノテーション情報を管理するクラスの入れ子構造は以下のようになっています．
// 各クラスの関数の詳細については alcon2015.hpp を御覧ください．
//
// The nested structure of classes to manage annotation information
// is shown as follows. As for the detail of functions of each class,
// please refer to the "alcon2015.hpp".
//
//
// prmu::ImageList : public std::list<prmu::Image> // 画像リスト
//  |
//  +-prmu::Image // 画像クラス
//    {
//    - std::string file_pass;              // ファイルパス
//    - prmu::vector<prmu::Logo> logo_list; // ロゴリスト
//       | 
//       +-prmu::Logo // ロゴクラス
//         {
//         - prmu::Label label; // ラベル
//         - prmu::Rect  bbox;  // 外接矩形
//         }
//    }
//

//
// ---------------------------------------------------------------------------------
//

int main( int narg, char *varg[] )
{
	// ---------- ここから環境に合わせて修正ください ----------

	// 応募レベル
	size_t lv = 1;

	// 画像データのパス，最後にスラッシュが必要
	// file path of images. '/' is needed at the end of the text.
	std::string fpath_root = "../../img/";

	// ---------- ここまで環境に合わせて修正ください ----------

	// アノテーションファイルの読み込み
	std::cout << "--> reading annotation files" << std::endl;

	// read annotation file
	prmu::ImageList imlist_learn;
	prmu::ImageList imlist_test[3];   // for Lv1~3  [0]==Lv1
	prmu::ImageList imlist_result[3]; // for Lv1~3  [0]==Lv1

	imlist_learn.fread_annotation( fpath_root + "learn/", "annotation.txt" );
	switch( lv )
	{
	case 3: imlist_test[2].fread_annotation( fpath_root + "test/lv3/", "annotation.txt" );
	case 2: imlist_test[1].fread_annotation( fpath_root + "test/lv2/", "annotation.txt" );
	case 1: imlist_test[0].fread_annotation( fpath_root + "test/lv1/", "annotation.txt" );
	}
	// 結果用画像リストへ、画像枚数やファイル名などの基本的な情報をコピー（ラベルや外接矩形情報は除く）
	// copy basic information to the output image lists
	// except for the label and the bounding box information
	switch( lv ) 
	{
	case 3: imlist_result[2].copy_basic_info( imlist_test[2] ); // for 文でも良かったが
	case 2: imlist_result[1].copy_basic_info( imlist_test[1] ); // 上述のファイル読み込みの書き方に
	case 1: imlist_result[0].copy_basic_info( imlist_test[0] ); // 合わせた
	}

	// -------------------------------

	// 処理開始
	std::cout << "--> processing user_function()" << std::endl;

	//// 時間計測用
	//auto start = std::chrono::system_clock::now(); 

	user_function( imlist_result, lv, imlist_learn, imlist_test );

	//// 時間計測用
	//auto end = std::chrono::system_clock::now();
	//auto dur = end - start;
	//auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();

	// -------------------------------

	// 点数計算
	std::cout << "--> scoring" << std::endl;

	switch( lv )
	{
	case 3: imlist_result[2].calc_score( imlist_test[2] ); std::cout << "   - Lv3, recall: " << imlist_result[2].score_total_recall << ",  precision: " << imlist_result[2].score_total_precision << std::endl;
	case 2: imlist_result[1].calc_score( imlist_test[1] ); std::cout << "   - Lv2, recall: " << imlist_result[1].score_total_recall << ",  precision: " << imlist_result[1].score_total_precision << std::endl;
	case 1: imlist_result[0].calc_score( imlist_test[0] ); std::cout << "   - Lv1, recall: " << imlist_result[0].score_total_recall << ",  precision: " << imlist_result[0].score_total_precision << std::endl;
	}

	// アノテーション情報の書き出し例
	std::cout << "--> writing results to files" << std::endl;

	switch( lv )
	{
	case 3: imlist_result[2].fwrite_annotation( fpath_root + "test/lv3/", "annotation_result.txt" );
	case 2: imlist_result[1].fwrite_annotation( fpath_root + "test/lv2/", "annotation_result.txt" );
	case 1: imlist_result[0].fwrite_annotation( fpath_root + "test/lv1/", "annotation_result.txt" );
	}

	return 0;
}

//
// ---------------------------------------------------------------------------------
//



