#ifndef __ALCON_2015_H__
#define __ALCON_2015_H__
//
// デバッグ用に書き換えることは可能とします．ただし，評価の際にはデフォルトのヘッダを用います．
// Modifying this file for debugging is allowed though the default header is used in evaluation.
//
//
// アノテーション情報を管理するクラスの入れ子構造は以下のようになっています．
// The nested structure of classes to manage annotation information is shown as follows.
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

#include <cstdlib>
#include <cstdio>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <list>
#include <vector>
#include <cmath>
#include <cfloat>
//#include <chrono>

#ifdef _WIN32
#pragma warning( disable : 4996 )
#endif

//
// ---------------------------------------------------------------------------------
// name space of PRMU
// ---------------------------------------------------------------------------------
//

namespace prmu
{
	// ラベル
	enum Label { // 異なる種類は 10 間隔を入れる（差の絶対値が 5 以上あるかどうかで区別）
		LABEL_UNKNOWN  =  0,
		LABEL_ASICS    = 10,
		LABEL_ASICS_a  = 11,
		LABEL_FILA     = 20,
		LABEL_FILA_f   = 21,
		// Le Coq Sportif
		LABEL_LECOQ    = 30,
		LABEL_MIZUNO   = 40,
		LABEL_MIZUNO_m = 41,
		// New Balance
		LABEL_NB    = 50,
		// New Balance
		LABEL_NB_n  = 51,
		// Shunsoku
		LABEL_SHUN  = 60,
		// Under Armour
		LABEL_UA    = 70,
		LABEL_YONEX = 80,
		// ------------ none-use ------------
		LABEL_NIKE  = 90,
		// Converse
		LABEL_CONV   = 100,
		LABEL_ADIDAS = 110,
		LABEL_REEBOK = 120,
		LABEL_PUMA   = 130,
		LABEL_KAPPA  = 140
	};

	namespace label
	{
		// 同種のラベルか
		inline bool issame( prmu::Label A, prmu::Label B )
		{
			return ( std::abs( A - B ) < 5 ) ? (true) : (false);
		}

		// -------------------------------

		// 文字列からラベルを探す
		// look for the corresponding label of a text string
		inline prmu::Label str2label( std::string str )
		{
			if      ( str == "ASICS"   ) return prmu::LABEL_ASICS;
			else if ( str == "ASICS_a" ) return prmu::LABEL_ASICS_a;
			else if ( str == "FILA"    ) return prmu::LABEL_FILA;
			else if ( str == "FILA_f"  ) return prmu::LABEL_FILA_f;
			else if ( str == "LECOQ"   ) return prmu::LABEL_LECOQ;
			else if ( str == "MIZUNO"  ) return prmu::LABEL_MIZUNO;
			else if ( str == "MIZUNO_m") return prmu::LABEL_MIZUNO_m;
			else if ( str == "NB"      ) return prmu::LABEL_NB;
			else if ( str == "NB_n"    ) return prmu::LABEL_NB_n;
			else if ( str == "SHUN"    ) return prmu::LABEL_SHUN;
			else if ( str == "UA"      ) return prmu::LABEL_UA;
			else if ( str == "YONEX"   ) return prmu::LABEL_YONEX;
			//------------ none-use ------------
			else if ( str == "NIKE"    ) return prmu::LABEL_NIKE;
			else if ( str == "CONV"    ) return prmu::LABEL_CONV;
			else if ( str == "ADIDAS"  ) return prmu::LABEL_ADIDAS;
			else if ( str == "REEBOK"  ) return prmu::LABEL_REEBOK;
			else if ( str == "PUMA"    ) return prmu::LABEL_PUMA;
			else if ( str == "KAPPA"   ) return prmu::LABEL_KAPPA;
			//
			else return prmu::LABEL_UNKNOWN;
		}

		// -------------------------------

		// ラベルから文字列を返す
		// look for the corresponding text string of a label
		inline std::string label2str( prmu::Label label )
		{
			if      ( label == prmu::LABEL_ASICS   ) return "ASICS";
			else if ( label == prmu::LABEL_ASICS_a ) return "ASICS_a";
			else if ( label == prmu::LABEL_FILA    ) return "FILA";
			else if ( label == prmu::LABEL_FILA_f  ) return "FILA_f";
			else if ( label == prmu::LABEL_LECOQ   ) return "LECOQ";
			else if ( label == prmu::LABEL_MIZUNO  ) return "MIZUNO";
			else if ( label == prmu::LABEL_MIZUNO_m) return "MIZUNO_m";
			else if ( label == prmu::LABEL_NB      ) return "NB";
			else if ( label == prmu::LABEL_NB_n    ) return "NB_n";
			else if ( label == prmu::LABEL_SHUN    ) return "SHUN";
			else if ( label == prmu::LABEL_UA      ) return "UA";
			else if ( label == prmu::LABEL_YONEX   ) return "YONEX";
			// ------------ none-use ------------
			else if ( label == prmu::LABEL_NIKE    ) return "NIKE";
			else if ( label == prmu::LABEL_CONV    ) return "CONV";
			else if ( label == prmu::LABEL_ADIDAS  ) return "ADIDAS";
			else if ( label == prmu::LABEL_REEBOK  ) return "REEBOK";
			else if ( label == prmu::LABEL_PUMA    ) return "PUMA";
			else if ( label == prmu::LABEL_KAPPA   ) return "KAPPA";
			//
			else return "UNKNOWN";
		}
	} // end of namespace label

	//
	// ---------------------------------------------------------------------------------
	//

	// クラスの前宣言
	// pre definition of classes

	template<class _Ty> class Point_;
	template<class _Ty> class Rect_;
	typedef Point_<int> Point;
	typedef Rect_<int> Rect;

	class Logo;
	class Image;

	//
	// ---------------------------------------------------------------------------------
	//

	template<class _Ty>
	std::ostream& operator << ( std::ostream& os, const prmu::Point_<_Ty>& pt );

	// 点クラス
	// point class to represent (x,y) coordinates of a point
	template<class _Ty>
	class Point_
	{
	public:
		typedef _Ty ty_value;

		// x coordinate
		_Ty x;
		// y coordinate
		_Ty y;

		// functions

		Point_() : x(0), y(0) {;}
		Point_( _Ty _x, _Ty _y ) : x(_x), y(_y) {;}
		Point_( const Point_& pt ) : x(pt.x), y(pt.y) {;}

		// -------------------------------

		// 点の座標を書き出す
		// write out the coordinates of a point
		friend std::ostream& operator << <_Ty> ( std::ostream& os, const prmu::Point_<_Ty>& pt );
	};

	// -------------------------------

	// 点の座標を書き出す
	// write out the coordinates of a point
	template<class _Ty>
	std::ostream& operator << ( std::ostream& os, const prmu::Point_<_Ty>& pt )
	{
		os << pt.x << ' ' << pt.y;
		return os;	
	}

	//
	// ---------------------------------------------------------------------------------
	//

	template<class _Ty>
	std::ostream& operator << ( std::ostream& os, const prmu::Rect_<_Ty>& bb  );

	// 矩形クラス
	// rectangle class
	template<class _Ty>
	class Rect_
	{
	public:
		typedef _Ty ty_value;

		// x coordinate of the left top point
		_Ty x;
		// y coordinate of the left top point
		_Ty y;
		// width of the bounding box (left - right + 1)
		_Ty w;
		// height of the bounding box (top - bottom + 1)
		_Ty h;

		// -- functions ------------------

		Rect_() : x(0), y(0), w(0), h(0) {;}
		Rect_( _Ty _x, _Ty _y, _Ty _w, _Ty _h ) : x(_x), y(_y), w(_w), h(_h) {;}
		Rect_( const Rect_& bb ) : x(bb.x), y(bb.y), w(bb.w), h(bb.h) {;}

		// -------------------------------

		// 矩形情報を登録
		// set the bounding box information
		void set( const Rect_& bb ) { x = bb.x;  y = bb.y;  w = bb.w;  h = bb.h; }
		void set( _Ty _x, _Ty _y, _Ty _w, _Ty _h ) { x =_x;  y =_y;  w =_w;  h =_h; }
		void set_using_center_pos( _Ty cx, _Ty cy, _Ty _w, _Ty _h ) { x = cx-w/2;  y = cy-h/2;  w =_w; h=_h;  }

		// -------------------------------

		// 中心座標を計算 (x+w/2, y+h/2)
		// calculate the center coordinates
		prmu::Point_<_Ty> pos_center() const { return prmu::Point_<_Ty>( x+w/_Ty(2), y+h/_Ty(2) ); }
		template<class _Tp2> void pos_center( _Tp2& cx, _Tp2& cy ) const { cx =_Tp2(x)+_Tp2(w)/_Tp2(2);  cy =_Tp2(y)+_Tp2(h)/_Tp2(2); }

		// -------------------------------

		// 右下端の座標を計算 (x+w-1, y+h-1)
		// calculate the bottom right coordinates
		prmu::Point_<_Ty> pos_bottom_right() const { return prmu::Point_<_Ty>( x+w-1, y+h-1 ); }
		template<class _Tp2> void pos_bottom_right( _Tp2& _x, _Tp2& _y ) const { _x=x+w-1;  _y=y+h-1; }

		// 面積を求める
		// area of this rect
		_Ty area() const { return w*h; }

		// -------------------------------

		// 他の矩形との重なりを求める
		// the overlapped rectangle between this and another rectangle
		Rect_ overlap( const Rect_& b ) const
		{
			_Ty _x, _y, _w, _h;
			_x = std::max( x, b.x );  _w = std::min( x+w, b.x+b.w ) -_x;
			_y = std::max( y, b.y );  _h = std::min( y+h, b.y+b.h ) -_y;
			return ( _w <= 0 || _h <= 0 ) ? (Rect_(0,0,0,0)) : (Rect_(_x,_y,_w,_h));
		}

		// -------------------------------

		// オーバーラップの割合を計算
		// overlap ratio between two rectangles
		double overlap_ratio( const Rect_& b ) const
		{
			Rect_ c = overlap( b );
			_Ty intrsct(0);
			if ( c.w > 0 && c.h > 0 )  intrsct = c.w * c.h;
			return (intrsct <= 0) ? (0.0) : (double(intrsct)/double(this->area()+b.area()-intrsct));
		}

		// -------------------------------

		// 外接矩形の情報を書き出す
		// write out the bounding box information
		friend std::ostream& operator << <_Ty> ( std::ostream& os, const prmu::Rect_<_Ty>& bb );

	}; // end of Rect

	// 外接矩形の情報を書き出す
	// write out the bounding box information
	template<class _Ty>
	std::ostream& operator << ( std::ostream& os, const prmu::Rect_<_Ty>& bb )
	{
		os << bb.x << ' ' << bb.y << ' ' << bb.w << ' ' << bb.h;
		return os;
	}


	//
	// ---------------------------------------------------------------------------------
	//

	// ロゴ情報
	// manage logo information
	class Logo
	{
	public:
		// ラベル
		prmu::Label label;

		// 外接矩形
		// bounding box
		prmu::Rect bbox;

		// コメント（適合率や再現率の表示用）
		// comment for displaying recall ratio and precision ratio
		std::string comment;

		// -- functions ------------------

		Logo() : label(prmu::LABEL_UNKNOWN) {;}

		// ラベルと矩形情報を指定して構築
		// generate with the label and the bounding box information
		Logo( prmu::Label _label, const prmu::Rect &_bbox ) : label(_label ), bbox(_bbox ) {;}
		Logo( prmu::Label _label, size_t x, size_t y, size_t w, size_t h ) : label(_label ), bbox(x,y,w,h) {;}

		// -------------------------------

		// ラベルと矩形情報を登録
		// set the label and the bounding box information
		void set( prmu::Label _label, size_t x, size_t y, size_t w, size_t h )
		{
			label = _label;  bbox.set(x,y,w,h);
		}
		void set( prmu::Label _label, const prmu::Rect& _bbox )
		{
			label = _label;  bbox.set(_bbox );
		}

		// -------------------------------

		// ラベルと外接矩形の情報を書き出す
		// write out the label and the bounding box information
		friend std::ostream& operator << ( std::ostream& os, const prmu::Logo& logo )
		{
			os << prmu::label::label2str( logo.label ) << ' ' << logo.bbox;
			if ( !logo.comment.empty() ) os << "\t% " << logo.comment;
			return os;
		}

		// -------------------------------

	}; // end of Logo

	//
	// ---------------------------------------------------------------------------------
	//

	// 画像中に含まれるロゴを管理
	// manage logos included in an image
	class Image
	{
	public:
		typedef std::vector<prmu::Logo> ty_logo_list;
		// ロゴリストのイテレータ
		// iterator for an image list
		typedef ty_logo_list::iterator ite_logo;
		typedef ty_logo_list::const_iterator c_ite_logo;

		// -- variables ------------------

		// ファイルパス
		std::string file_path;
		// ファイル名
		std::string file_name;

		// ロゴのリスト
		ty_logo_list logo_list;

		// スコア 適合率
		double score_precision;

		// スコア 再現率
		double score_recall;

		// コメント（適合率や再現率の表示用）
		// comment for displaying recall ratio and precision ratio
		std::string comment;

		// -- functions ------------------

		Image() : score_precision(0.0), score_recall(0.0) { ; }

		// -------------------------------

		// 画像ファイルへのフルパス
		std::string full_file_path() const
		{
			return file_path + file_name;
		}

		// -------------------------------

		// ロゴのリストにラベル情報を追加
		void append_result( prmu::Label label, prmu::Rect rect )
		{
			logo_list.push_back( Logo( label, rect ) );
		}
		void append_result( prmu::Label label, size_t x, size_t y, size_t w, size_t h )
		{
			logo_list.push_back( Logo( label, x, y, w, h ) );
		}

		// -------------------------------

		// 学習用ロゴ画像の矩形情報を取得
		// (学習用の画像ではロゴの数は一つであり，ロゴリストの最初の情報を返す)
		// return the bounding box information of the image used for learning
		// (the image for learning includes a single logo, and therefore the first element of the list is returned)
		const prmu::Rect rect_of_1st_img() const
		{
			return logo_list[0].bbox;
		}
		// -------------------------------

		// 学習用ロゴ画像のラベル情報を取得
		// (学習用の画像ではロゴの数は一つであり，ロゴリストの最初の情報を返す)
		// return the label information of the image used for learning
		// (the image for learning includes a single logo, and therefore the first element of the list is returned)
		const prmu::Label label_of_1st_img() const
		{
			return logo_list[0].label;
		}

		// -------------------------------

		// 登録されているロゴの個数を返す
		// return the number of registered logos in this image
		size_t num_logo() const
		{
			return logo_list.size();
		}

		// -------------------------------

		// ロゴリストを初期化
		// clear the logo list
		void clear()
		{
			logo_list.clear();
		}

		// -------------------------------

		// 画像中の k 番目のロゴ情報を習得
		// get the pointer of k'th logo in this image
		prmu::Logo* get_logo( size_t k )
		{
			ite_logo ite = logo_list.begin();
			for ( size_t i = 0; i < k; ++i )  { ++ite; }
			return &*ite;
		}

		// -------------------------------

		// もっとも近い矩形をもつロゴを探し，ポインタを返す
		// look for the nearest logo by rectangle, and return its pointer
		const prmu::Logo* lookfor_nearest_logo( const prmu::Rect& rect ) const
		{
			const prmu::Logo* plogo = NULL;
			double px, py, qx, qy, dx, dy, dd, dd_min=DBL_MAX;
			rect.pos_center( px, py );

			for ( c_ite_logo ite = logo_list.begin(); ite != logo_list.end(); ++ite )
			{
				ite->bbox.pos_center( qx, qy );
				dx = px - qx;  dy = py - qy;
				dd = dx*dx + dy*dy;
				if ( dd < dd_min )
				{
					dd_min = dd;
					plogo = &*ite;
				}
				// ほぼないであろうが，距離が同じ場合，ラベル値の小さなものを優先
				else if ( dd == dd_min && plogo->label < ite->label )
				{
					dd_min = dd;
					plogo = &*ite;
				}
			}
			return plogo;
		}
		// 非const版
		prmu::Logo* lookfor_nearest_logo( const prmu::Rect& rect )
		{
			return const_cast<prmu::Logo*>(
				static_cast<const Image&>(*this).lookfor_nearest_logo( rect ) );
		}

		// -------------------------------

		// 画像内に含まれるロゴ情報を書き出す
		// print out all logo information contained in an image
		friend std::ostream& operator << ( std::ostream& os, const prmu::Image& img )
		{
			os << '%' << std::endl;
			os << img.file_name << std::endl;
			os << img.num_logo() << std::endl;
			for ( prmu::Image::c_ite_logo ite = img.logo_list.begin();  ite != img.logo_list.end(); ++ite )
			{
				os << *ite << std::endl;
			}
			if ( !img.comment.empty() ) os << "% " << img.comment << std::endl;
			return os;
		}
	}; // end of Image class


	//
	// ---------------------------------------------------------------------------------
	//

	// 画像ファイル名とアノテーション情報を管理
	// manage the file name and the annotation information of each image
	class ImageList : public std::list<prmu::Image>
	{
	public:
		// 画像リストのイテレータ
		// iterator for an image list
		typedef ImageList::iterator ite_img;
		typedef ImageList::const_iterator c_ite_img;

		// スコア 適合率
		double score_total_precision;

		// スコア 再現率
		double score_total_recall;

		// コメント
		std::string comment;

		// -------------------------------

		// 画像枚数やファイル名などの基本的な情報をコピー
		// copy basic information except for the label and the bounding box information
		void copy_basic_info( const ImageList& gtruth )
		{
			this->resize( gtruth.size() );

			c_ite_img ite_g = gtruth.begin();
			ite_img ite = this->begin();

			for ( ; ite != this->end(); ++ite, ++ite_g )
			{
				ite->file_path = ite_g->file_path;
				ite->file_name = ite_g->file_name;
				ite->logo_list.reserve( ite_g->logo_list.size() );
			}
		}

		// -------------------------------

		// 指定した画像ファイル名に一致するアノテーション情報を探す
		// look for the information (i.e., pointer) of an image file by specifying its file pass
		// from an annotation file.
		prmu::Image* lookfor_image( const std::string& fname )
		{
			for ( ite_img ite = this->begin(); ite != this->end(); ++ite )
			{
				if ( ite->file_name == fname )  return &*ite;
			}
			return NULL; // cannot find
		}

		// -------------------------------

		// アノテーション情報を読み込む
		// read in annotation information
		int fread_annotation( const std::string& fpath, const std::string& fname )
		{
			std::ifstream fin;
			fin.open( (fpath+fname).c_str(), std::ios::in );
			if ( fin.fail() )
			{
				std::cerr << "[ error ] cannot open the annotation file : " << fname << std::endl;
				return -1;
			}
			std::string line, label;
			prmu::Image img;
			prmu::Logo logo;
			size_t cnt;
			enum mode_{READ_FILE_NAME, READ_NUM_OBJ, READ_LABEL_AND_RECT} mode = READ_FILE_NAME;

			while ( std::getline( fin, line ) )
			{
				if ( !line.length() || line[0] == '%' ) continue; // comment skip

				std::stringstream iss( line, std::istringstream::in );

				switch ( mode )
				{
				case READ_FILE_NAME: // image file name
					img.file_path = fpath;
					iss >> img.file_name;
					mode = READ_NUM_OBJ;  break;

				case READ_NUM_OBJ: // number of objects
					iss >> cnt;
					img.logo_list.reserve( cnt );
					mode = READ_LABEL_AND_RECT;  break;

				case READ_LABEL_AND_RECT: // label and bounding box information
					if ( cnt > 0 )
					{
						iss >> label >> logo.bbox.x >> logo.bbox.y >> logo.bbox.w >> logo.bbox.h;
						logo.label = prmu::label::str2label( label );
						img.logo_list.push_back( logo );
					}
					if (--cnt == 0 )
					{
						this->push_back( img );
						img.clear();
						mode = READ_FILE_NAME;
					}
					break;
				}
			}
			fin.close();

			return 0;
		}

		// -------------------------------

		// アノテーション情報を書き出す
		// write out annotation information
		int fwrite_annotation( const std::string& fpath, const std::string& fname )
		{
			std::ofstream fout;
			fout.open( (fpath+fname).c_str(), std::ios::out );
			if ( fout.fail() )
			{
				std::cerr << "[ error ] cannot make the annotation file : " << fname << std::endl;
				return -1;
			}
			for ( c_ite_img ite = this->begin();  ite != this->end(); ++ite )
			{
				fout << *ite;
			}
			if ( !this->comment.empty() ) fout << "% " << this->comment << std::endl;
			fout.close();

			return 0;
		}

		// -------------------------------

		// 正解のアノテーション情報と識別結果の情報とのスコアリング
		// take the score (precision and recall) of each image
		void calc_score( const prmu::ImageList& gtruth )
		{
			prmu::ImageList::c_ite_img ite_g = gtruth.begin();
			prmu::ImageList::ite_img ite_r = begin();
			prmu::Image::c_ite_logo jte_g;
			prmu::Image::ite_logo jte_r;
			const prmu::Image::ty_logo_list* logos_g;
			prmu::Image::ty_logo_list* logos_r;

			double scr_total_r(0.0), scr_total_p(0.0); // total score
			size_t num_total_r(0),   num_total_p(0);

			for ( ;  ite_g != gtruth.end();  ++ite_g, ++ite_r )
			{
				double scr_r(0.0), scr_p(0.0); // recall and precision
				logos_g = &ite_g->logo_list;
				logos_r = &ite_r->logo_list;

				if ( !logos_r->empty() )
				{
					// recall
					for ( jte_g = logos_g->begin(); jte_g != logos_g->end(); ++jte_g )
					{
						prmu::Logo* plogo = ite_r->lookfor_nearest_logo( jte_g->bbox );
						scr_r += ( prmu::label::issame( jte_g->label, plogo->label ) ) ?
							( 100.0*jte_g->bbox.overlap_ratio( plogo->bbox ) ) : (0.0);
					}
					// precision
					for ( jte_r = logos_r->begin(); jte_r != logos_r->end(); ++jte_r )
					{
						const prmu::Logo* plogo = ite_g->lookfor_nearest_logo( jte_r->bbox );
						scr_p += ( prmu::label::issame( jte_r->label, plogo->label ) ) ?
							( 100.0*jte_r->bbox.overlap_ratio( plogo->bbox ) ) : (0.0);
					}
				}
				// recall and precision at each image
				ite_r->score_recall = scr_r / double( logos_g->size() );
				if ( logos_r->size() ) // avoid division by zero
					ite_r->score_precision = scr_p / double( logos_r->size() ); 

				// for total recall and precision
				scr_total_r += scr_r;    num_total_r += logos_g->size();
				scr_total_p += scr_p;    num_total_p += logos_r->size();

				std::stringstream ss; ss << "recall " << ite_r->score_recall << ' ' << "precision " << ite_r->score_precision;
				ite_r->comment = ss.str();
			}
			this->score_total_recall = scr_total_r / double(num_total_r);
			this->score_total_precision = scr_total_p / double(num_total_p);

			std::stringstream ss;
			ss << std::endl;
			ss << "% --> total recall    " << this->score_total_recall << std::endl;
			ss << "% --> total precision " << this->score_total_precision;
			this->comment = ss.str();
		}
	};

} // end of namespace prmu

//
// ---------------------------------------------------------------------------------
//

// コンテスト用の関数
// function developed by a contestant
void user_function(
	//
	// output
	prmu::ImageList (&imlist_result)[3],
	//
	// input
	size_t lv,
	const prmu::ImageList& imlist_learn,
	const prmu::ImageList (&imlist_test)[3]
);

#endif // __ALCON_2015_H__

