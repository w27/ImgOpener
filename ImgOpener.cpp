#include <windows.h>
#include <gdiplus.h>
#include <windowsx.h>
#include <string>

#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

std::wstring g_imagePath;
bool g_dragging = false; 
POINT g_mousePos;        


LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static Image* image = nullptr;

    switch (msg) {
    case WM_CREATE: {
        if (!g_imagePath.empty()) {
            image = new Image(g_imagePath.c_str());
            if (image->GetLastStatus() != Ok) {
                MessageBox(hwnd, L"Error", L"Error", MB_OK | MB_ICONERROR);
                delete image;
                image = nullptr;
            }
            else {
                UINT width = image->GetWidth();
                UINT height = image->GetHeight();

                SetWindowPos(hwnd, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
            }
        }
        break;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        if (image) {
            Graphics graphics(hdc);
            graphics.DrawImage(
                image,
                0, 0,
                static_cast<INT>(image->GetWidth()), 
                static_cast<INT>(image->GetHeight()) 
            );
        }

        EndPaint(hwnd, &ps);
        break;
    }

    case WM_LBUTTONDOWN: {
        g_dragging = true;
        g_mousePos.x = GET_X_LPARAM(lParam);
        g_mousePos.y = GET_Y_LPARAM(lParam);
        SetCapture(hwnd);
        break;
    }

    case WM_MOUSEMOVE: {
        if (g_dragging) {
            // Перемещаем окно
            POINT newPos;
            GetCursorPos(&newPos);
            SetWindowPos(hwnd, NULL, newPos.x - g_mousePos.x, newPos.y - g_mousePos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        }
        break;
    }

    case WM_LBUTTONUP: {
        if (g_dragging) {
            g_dragging = false;
            ReleaseCapture(); 
        }
        break;
    }

    case WM_RBUTTONDOWN: {
        PostQuitMessage(0);
        break;
    }

    case WM_SETCURSOR: {
        if (LOWORD(lParam) == HTCLIENT) {
            SetCursor(LoadCursor(NULL, IDC_ARROW));
            return TRUE;
        }
        break;
    }

    case WM_DESTROY:
        if (image) {
            delete image;
        }
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argv && argc > 1) {
        g_imagePath = argv[1];
    }
    LocalFree(argv);

    const wchar_t CLASS_NAME[] = L"SampleWindowClass";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,                             
        CLASS_NAME,                    
        NULL,                           
        WS_POPUP,                      
        CW_USEDEFAULT, CW_USEDEFAULT,  
        800, 600,                       
        NULL,                          
        NULL,                           
        hInstance,                     
        NULL                           
    );

    if (hwnd == NULL) {
        GdiplusShutdown(gdiplusToken);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    GdiplusShutdown(gdiplusToken);

    return 0;
}
