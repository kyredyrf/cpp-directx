#include "windows/windows-app.h"

/**
 *  アプリ
 */
class App : public WindowsApp
{
	struct Private;
	normal_unique_ptr<Private> p;

public:
	App(int screenWidth, int screenHeight);
	~App();

protected:
	void OnSetup() override;
	void OnCleanup() override;
	void OnUpdate() override;
};
