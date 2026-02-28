#include "platform/windows/splash_screen.h"

#include <algorithm>
#include <chrono>
#include <future>
#include <string>
#include <windows.h>
#include <vector>

#include "core/layout_discovery.h"
#include "lib/stb_image.h"


namespace bijoy::platform::windows {
    namespace {

        constexpr wchar_t kSplashClassName[] = L"OmorEkusheSplash";
        constexpr UINT_PTR kSplashTimerId = 1;
        constexpr int kTickMs = 10;
        constexpr BYTE kFadeStepAlpha = 6;
        constexpr int kMinVisibleMs = 5000;

        constexpr int kWindowWidth = 350;
        constexpr int kWindowHeight = 450;

        class SplashScreenController {
        public:
            SplashScreenController(SplashInitTask initTask,
                                   SplashCompletedCallback onCompleted)
                    : initTask_(std::move(initTask)),
                      onCompleted_(std::move(onCompleted)) {}

            HWND Create(HINSTANCE instance, HWND owner) {
                RegisterWindowClass(instance);

                hwnd_ = CreateWindowExW(
                        WS_EX_LAYERED | WS_EX_TOPMOST,
                        kSplashClassName,
                        L"Splash",
                        WS_POPUP,
                        0, 0,
                        kWindowWidth,
                        kWindowHeight,
                        owner,
                        nullptr,
                        instance,
                        this);

                return hwnd_;
            }

        private:
            static void RegisterWindowClass(HINSTANCE instance) {
                static bool registered = false;
                if (registered) return;

                WNDCLASSEXW wc{};
                wc.cbSize = sizeof(wc);
                wc.lpfnWndProc = WndProc;
                wc.hInstance = instance;
                wc.lpszClassName = kSplashClassName;
                wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
                wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
                RegisterClassExW(&wc);

                registered = true;
            }

            static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
                auto* self = reinterpret_cast<SplashScreenController*>(
                        GetWindowLongPtrW(hwnd, GWLP_USERDATA));

                if (msg == WM_NCCREATE) {
                    auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
                    auto* controller = static_cast<SplashScreenController*>(cs->lpCreateParams);
                    SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(controller));
                    controller->hwnd_ = hwnd;
                    return TRUE;
                }

                if (!self) {
                    return DefWindowProcW(hwnd, msg, wParam, lParam);
                }

                switch (msg) {
                    case WM_CREATE:   return self->OnCreate();
                    case WM_TIMER:    self->OnTimer(); return 0;
                    case WM_PAINT:    self->OnPaint(); return 0;
                    case WM_DESTROY:  KillTimer(hwnd, kSplashTimerId); return 0;
                    case WM_NCDESTROY:
                        if (self->backgroundBitmap_) {
                            DeleteObject(self->backgroundBitmap_);
                        }
                        self->OnDestroyed();
                        delete self;
                        return 0;
                    default:
                        return DefWindowProcW(hwnd, msg, wParam, lParam);
                }
            }

            LRESULT OnCreate() {
                alpha_ = 0;
                LoadBackgroundImage();
                UpdateWindowAlpha(0);
                CenterOnScreen();

                startTime_ = std::chrono::steady_clock::now();

                if (initTask_) {
                    initFuture_ = std::async(std::launch::async, initTask_);
                } else {
                    initCompleted_ = true;
                }

                SetTimer(hwnd_, kSplashTimerId, kTickMs, nullptr);
                ShowWindow(hwnd_, SW_SHOW);
                UpdateWindow(hwnd_);
                return 0;
            }

            void OnTimer() {
                if (!initCompleted_ &&
                    initFuture_.valid() &&
                    initFuture_.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                    initFuture_.get();
                    initCompleted_ = true;
                }

                if (alpha_ < 255) {
                    alpha_ = static_cast<BYTE>(std::min(255, alpha_ + kFadeStepAlpha));
                    UpdateWindowAlpha(alpha_);
                    return;
                }

                const auto elapsedMs =
                        std::chrono::duration_cast<std::chrono::milliseconds>(
                                std::chrono::steady_clock::now() - startTime_).count();

                if (initCompleted_ && elapsedMs >= kMinVisibleMs) {
                    DestroyWindow(hwnd_);
                }
            }

            void OnPaint() {
                PAINTSTRUCT ps;
                BeginPaint(hwnd_, &ps);
                EndPaint(hwnd_, &ps);
            }

            void LoadBackgroundImage() {
                const std::wstring appDir = bijoy::core::GetAppDirectory();
                const std::wstring candidates[] = {
                    appDir + L"data\\splash.png",
                    appDir + L"..\\data\\splash.png",
                    appDir + L"splash.png"
                };

                std::wstring resolvedPath;
                for (const auto& candidate : candidates) {
                    if (GetFileAttributesW(candidate.c_str()) != INVALID_FILE_ATTRIBUTES) {
                        resolvedPath = candidate;
                        break;
                    }
                }

                if (resolvedPath.empty()) return;

                // Convert wstring path to string for stb_image
                int len = WideCharToMultiByte(CP_UTF8, 0, resolvedPath.c_str(), -1, nullptr, 0, nullptr, nullptr);
                if (len <= 0) return;
                std::string pathUtf8(len - 1, '\0');
                WideCharToMultiByte(CP_UTF8, 0, resolvedPath.c_str(), -1, &pathUtf8[0], len, nullptr, nullptr);

                int width, height, channels;
                stbi_uc* data = stbi_load(pathUtf8.c_str(), &width, &height, &channels, 4);
                if (!data) return;

                BITMAPINFO bmi = {};
                bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                bmi.bmiHeader.biWidth = width;
                bmi.bmiHeader.biHeight = -height; // Top-down
                bmi.bmiHeader.biPlanes = 1;
                bmi.bmiHeader.biBitCount = 32;
                bmi.bmiHeader.biCompression = BI_RGB;

                HDC screenDc = GetDC(nullptr);
                void* bits = nullptr;
                backgroundBitmap_ = CreateDIBSection(screenDc, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
                if (backgroundBitmap_ && bits) {
                    // Convert RGBA to BGRA and Pre-multiply Alpha
                    auto* pixels = static_cast<unsigned char*>(bits);
                    for (int i = 0; i < width * height; ++i) {
                        unsigned char a = data[i * 4 + 3];
                        pixels[i * 4 + 0] = (data[i * 4 + 2] * a) / 255; // B
                        pixels[i * 4 + 1] = (data[i * 4 + 1] * a) / 255; // G
                        pixels[i * 4 + 2] = (data[i * 4 + 0] * a) / 255; // R
                        pixels[i * 4 + 3] = a;                           // A
                    }
                }
                ReleaseDC(nullptr, screenDc);
                stbi_image_free(data);
            }

            void UpdateWindowAlpha(BYTE alpha) {
                if (!backgroundBitmap_) return;

                RECT rect;
                GetWindowRect(hwnd_, &rect);
                SIZE size = { kWindowWidth, kWindowHeight };
                POINT ptSrc = { 0, 0 };
                POINT ptDest = { rect.left, rect.top };

                HDC screenDc = GetDC(nullptr);
                HDC memDc = CreateCompatibleDC(screenDc);
                HGDIOBJ oldBitmap = SelectObject(memDc, backgroundBitmap_);

                BLENDFUNCTION blend = { AC_SRC_OVER, 0, alpha, AC_SRC_ALPHA };

                UpdateLayeredWindow(hwnd_, screenDc, &ptDest, &size, memDc, &ptSrc, 0, &blend, ULW_ALPHA);

                SelectObject(memDc, oldBitmap);
                DeleteDC(memDc);
                ReleaseDC(nullptr, screenDc);
            }

            void OnDestroyed() {
                if (onCompleted_) {
                    onCompleted_();
                }
            }

            void CenterOnScreen() {
                const int x = (GetSystemMetrics(SM_CXSCREEN) - kWindowWidth) / 2;
                const int y = (GetSystemMetrics(SM_CYSCREEN) - kWindowHeight) / 2;
                SetWindowPos(hwnd_, HWND_TOPMOST, x, y, kWindowWidth, kWindowHeight, SWP_SHOWWINDOW);
            }

        private:
            HWND hwnd_ = nullptr;
            BYTE alpha_ = 0;
            bool initCompleted_ = false;
            HBITMAP backgroundBitmap_ = nullptr;

            SplashInitTask initTask_;
            SplashCompletedCallback onCompleted_;
            std::future<void> initFuture_;
            std::chrono::steady_clock::time_point startTime_;
        };

    } // namespace

    HWND ShowSplashScreen(HINSTANCE hInstance,
                          HWND owner,
                          SplashInitTask initTask,
                          SplashCompletedCallback onCompleted) {
        auto* controller =
                new SplashScreenController(std::move(initTask), std::move(onCompleted));
        return controller->Create(hInstance, owner);
    }

} // namespace bijoy::platform::windows