#include "windows-app.h"
#include "windows-app-private.h"
#include <memory>
#include <tchar.h>

/**
 *	Windows アプリのエントリーポイント
 */
int WINAPI _tWinMain(
	HINSTANCE hInstance,
	HINSTANCE,
	LPTSTR,
	int nCmdShow
)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	new_normal<int>();
	// _CrtSetDbgFlag の設定を行っておくとアプリ終了時に以下のようなログが出力されリーク箇所が即座に分かる
	// ----
	// Detected memory leaks!
	// Dumping objects ->
	// windows-app.cpp(17) : {76} normal block at 0x000001F0982884A0, 4 bytes long.
	// Data : < > 00 00 00 00
	// Object dump complete.
	// ----

	normal_unique_ptr<WindowsApp> app = CreateWindowsApp();
	return app->Run(hInstance, nCmdShow);
}

/**
 *	ウィンドウプロシージャ
 */
LRESULT WindowsApp::WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	auto app = reinterpret_cast<WindowsApp*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	if (app != nullptr)
	{
		return app->OnWindowProc(hWnd, Msg, wParam, lParam);
	}

	switch (Msg)
	{
	case WM_CREATE:
	{
		auto pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
		app = reinterpret_cast<WindowsApp*>(pCreateStruct->lpCreateParams);
		app->p->hWnd = hWnd;
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
		return app->OnWindowProc(hWnd, Msg, wParam, lParam);
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProc(hWnd, Msg, wParam, lParam);
	}
}

/**
 *	ウィンドウプロシージャ
 */
LRESULT WindowsApp::OnWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg)
	{
	case WM_CREATE:
		OnSetup();
		SetTimer(hWnd, 1, 0, nullptr);
		return 0;
	case WM_DESTROY:
		KillTimer(hWnd, 1);
		OnCleanup();
		PostQuitMessage(0);
		return 0;
	case WM_TIMER:
		InvalidateRect(hWnd, nullptr, FALSE);
		return 0;
	case WM_PAINT:
		OnUpdate();
		return 0;
	default:
		return DefWindowProc(hWnd, Msg, wParam, lParam);
	}
}

/**
 *	コンストラクタ
 *	@param screenWidth 画面の幅
 *	@param screenHeight 画面の高さ
 */
WindowsApp::WindowsApp(int screenWidth, int screenHeight)
{
	p = make_unique<Private>(screenWidth, screenHeight);
}

/**
 *	デストラクタ
 */
WindowsApp::~WindowsApp()
{

}

/**
 *	アプリを実行
 *	@param hInstance インスタンスハンドル
 *	@param nCmdShow ウィンドウの初期表示
 */
int WindowsApp::Run(HINSTANCE hInstance, int nCmdShow) const
{
	// ウィンドウクラスを登録
	WNDCLASSEX windowClass = { 0 };
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = &WindowProc;
	windowClass.hInstance = hInstance;
	windowClass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
	windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	windowClass.lpszClassName = _T("cpp-directx");
	RegisterClassEx(&windowClass);

	// 画面サイズがちょうど収まる大きさのウィンドウサイズを計算
	RECT windowRect = { 0, 0, static_cast<LONG>(p->screenWidth), static_cast<LONG>(p->screenHeight) };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	// ウィンドウを作成
	CreateWindow(
		windowClass.lpszClassName,
		windowClass.lpszClassName,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		nullptr,
		nullptr,
		hInstance,
		const_cast<WindowsApp*>(this)
	);

	// ウィンドウを表示
	ShowWindow(p->hWnd, nCmdShow);
	UpdateWindow(p->hWnd);

	// メッセージループ
	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return static_cast<char>(msg.wParam);
}

/**
 *	ウィンドウハンドルを取得
 */
HWND WindowsApp::GetWindowsHandle() const
{
	return p->hWnd;
}

/**
 *	画面サイズを取得
 */
std::tuple<int, int> WindowsApp::GetScreenSize() const
{
	return { p->screenWidth, p->screenHeight };
}
