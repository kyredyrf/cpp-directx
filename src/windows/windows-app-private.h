#pragma once

/**
 * Windows アプリ非公開データ
 */
struct WindowsApp::Private
{
	Private(int screenWidth, int screenHeight);
	~Private();

	const int screenWidth;
	const int screenHeight;

	HWND hWnd;
};

/**
 *	コンストラクタ
 * @param screenWidth 画面の幅
 * @param screenHeight 画面の高さ
 */
WindowsApp::Private::Private(int screenWidth, int screenHeight)
	: screenWidth(screenWidth)
	, screenHeight(screenHeight)
{
}

/**
 *	デストラクタ
 */
WindowsApp::Private::~Private()
{

}
