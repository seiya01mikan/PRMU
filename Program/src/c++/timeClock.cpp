#include <iostream>
#include <ctime>
using namespace std;

class Timer {
	double begin, end;
public:
	Timer();   // コンストラクタ
	~Timer();  // デストラクタ
	void start();
	void stop();
	void show();
};

Timer::Timer()
{
	begin = end = 0.0;  // 初期化
}

Timer::~Timer()
{
	cout << "Timerオブジェクトの破棄" << endl;
}

void Timer::start()
{
	begin = (double)clock() / CLOCKS_PER_SEC;
}

void Timer::stop()
{
	end = (double)clock() / CLOCKS_PER_SEC;
}

void Timer::show()
{
	cout << "処理時間：" << end - begin << endl;
}
