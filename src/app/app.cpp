#include "app.h"
#include "app-private.h"

/**
 *	Windows アプリを作成
 */
normal_unique_ptr<WindowsApp> CreateWindowsApp()
{
    return normal_unique_ptr<WindowsApp>(new_normal<App>(640, 360));
}

/**
 *	コンストラクタ
 * @param screenWidth 画面の幅
 * @param screenHeight 画面の高さ
 */
App::App(int screenWidth, int screenHeight)
    : WindowsApp(screenWidth, screenHeight)
{
    p = make_unique<Private>(this);
}

/**
 *	デストラクタ
 */
App::~App()
{
}

/**
 *	セットアップ
 */
void App::OnSetup()
{
    p->LoadPipeline();
    p->LoadAssets();
}

/**
 *	クリーンアップ
 */
void App::OnCleanup()
{
    p->WaitForPreviousFrame();

    CloseHandle(p->m_fenceEvent);
}

/**
 *	アップデート
 */
void App::OnUpdate()
{
    // Record all the commands we need to render the scene into the command list.
    p->PopulateCommandList();

    // Execute the command list.
    ID3D12CommandList* ppCommandLists[] = { p->m_commandList.Get() };
    p->m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Present the frame.
    ThrowIfFailed(p->m_swapChain->Present(1, 0));

    p->WaitForPreviousFrame();
}
