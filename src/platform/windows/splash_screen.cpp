#include "platform/windows/splash_screen.h"

#include <algorithm>
#include <chrono>
#include <future>
#include <string>
#include <windows.h>


namespace bijoy::platform::windows {
    namespace {

        constexpr wchar_t kSplashClassName[] = L"BijoyBayannoSplash";
        constexpr UINT_PTR kSplashTimerId = 1;
        constexpr int kTickMs = 10;
        constexpr BYTE kFadeStepAlpha = 6;
        constexpr int kMinVisibleMs = 1200;

        constexpr int kWindowWidth = 360;
        constexpr int kWindowHeight = 260;

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
                        self->OnDestroyed();
                        delete self;
                        return 0;
                    default:
                        return DefWindowProcW(hwnd, msg, wParam, lParam);
                }
            }

            LRESULT OnCreate() {
                alpha_ = 0;
                SetLayeredWindowAttributes(hwnd_, 0, alpha_, LWA_ALPHA);
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
                    SetLayeredWindowAttributes(hwnd_, 0, alpha_, LWA_ALPHA);
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
                HDC hdc = BeginPaint(hwnd_, &ps);

                RECT rect;
                GetClientRect(hwnd_, &rect);

                // Gradient background
                TRIVERTEX vertices[2] = {
                        {0, 0, 230 << 8, 235 << 8, 240 << 8, 0},
                        {rect.right, rect.bottom, 255 << 8, 255 << 8, 255 << 8, 0}
                };
                GRADIENT_RECT gRect = {0, 1};
                GradientFill(hdc, vertices, 2, &gRect, 1, GRADIENT_FILL_RECT_V);

                // Card
                RECT card = {20, 20, rect.right - 20, rect.bottom - 20};
                HBRUSH cardBrush = CreateSolidBrush(RGB(255, 255, 255));
                FillRect(hdc, &card, cardBrush);
                DeleteObject(cardBrush);

                SetBkMode(hdc, TRANSPARENT);
                SetTextColor(hdc, RGB(32, 32, 32));

                HFONT titleFont = CreateFontW(
                        34, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

                HFONT subtitleFont = CreateFontW(
                        16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

                HFONT oldFont = static_cast<HFONT>(SelectObject(hdc, titleFont));

                RECT titleRect = card;
                titleRect.top += 50;
                DrawTextW(hdc, L"বিজয়", -1, &titleRect, DT_CENTER | DT_SINGLELINE);

                SelectObject(hdc, subtitleFont);
                RECT subtitleRect = card;
                subtitleRect.top += 100;
                DrawTextW(hdc, L"Bijoy Bayanno", -1, &subtitleRect, DT_CENTER | DT_SINGLELINE);

                SelectObject(hdc, oldFont);
                DeleteObject(titleFont);
                DeleteObject(subtitleFont);

                EndPaint(hwnd_, &ps);
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