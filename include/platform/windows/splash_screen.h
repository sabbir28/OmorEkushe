#pragma once

#include <functional>
#include <windows.h>

namespace bijoy::platform::windows {

using SplashInitTask = std::function<void()>;
using SplashCompletedCallback = std::function<void()>;

HWND ShowSplashScreen(HINSTANCE hInstance,
                      HWND owner,
                      SplashInitTask initTask,
                      SplashCompletedCallback onCompleted);

} // namespace bijoy::platform::windows
