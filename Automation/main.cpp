
#include <windows.h>
#include <shellapi.h>
#include <thread>
#include <atomic>
#include <uxtheme.h>
#pragma comment(lib, "uxtheme.lib")
#include "resource.h"


#define WMAPP_NOTIFYCALLBACK (WM_APP + 1)

HINSTANCE hInst;
HWND hWndMain;
NOTIFYICONDATA nid = {};
std::atomic<bool> running(false);

void MouseClick() {
    INPUT inputs[2] = {};
    inputs[0].type = INPUT_MOUSE;
    inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    inputs[1].type = INPUT_MOUSE;
    inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(2, inputs, sizeof(INPUT));
}

void ClickLoop() {
    const int intervalSeconds = 180;
    POINT lastPos, currentPos;
    DWORD unchangedTime = 0;

    GetCursorPos(&lastPos);
    DWORD startTime = GetTickCount();

    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        GetCursorPos(&currentPos);

        if (currentPos.x == lastPos.x && currentPos.y == lastPos.y) {
            unchangedTime += 100;
            if (unchangedTime >= intervalSeconds * 1000) {
                MouseClick();
                startTime = GetTickCount();
                unchangedTime = 0;
            }
        }
        else {
            unchangedTime = 0;
            lastPos = currentPos;
            startTime = GetTickCount();
        }
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HWND hButton;

    switch (msg) {
    case WM_CREATE:
        hButton = CreateWindow(L"BUTTON", L"Start", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            60, 40, 160, 40, hWnd, (HMENU)1, hInst, NULL);
        SetWindowTheme(hButton, L"", L"");

        nid.cbSize = sizeof(NOTIFYICONDATA);
        nid.hWnd = hWnd;
        nid.uID = 1;
        nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        nid.uCallbackMessage = WMAPP_NOTIFYCALLBACK;
        nid.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));

        wcscpy_s(nid.szTip, L"Automation");
        Shell_NotifyIcon(NIM_ADD, &nid);
        return 0;

    case WM_COMMAND:
        if (LOWORD(wParam) == 1) {
            if (!running) {
                SetWindowText(hButton, L"Stop");
                running = true;
                std::thread(ClickLoop).detach();
            }
            else {
                SetWindowText(hButton, L"Start");
                running = false;
            }
        }
        return 0;

    case WMAPP_NOTIFYCALLBACK:
        if (LOWORD(lParam) == WM_RBUTTONUP) {
            PostMessage(hWnd, WM_CLOSE, 0, 0);
        }
        return 0;

    case WM_NCHITTEST:
        return HTCAPTION;

    case WM_CLOSE:
        running = false;
        Shell_NotifyIcon(NIM_DELETE, &nid);
        DestroyWindow(hWnd);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    hInst = hInstance;

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = L"Automation";

    RegisterClass(&wc);

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int windowWidth = 300;
    int windowHeight = 150;
    int xPos = (screenWidth - windowWidth) / 2;
    int yPos = (screenHeight - windowHeight) / 2;

    hWndMain = CreateWindowEx(0, wc.lpszClassName, L"Automation",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        xPos, yPos,  // 修改位置参数
        windowWidth, windowHeight,
        NULL, NULL, hInstance, NULL);

    ShowWindow(hWndMain, nCmdShow);
    UpdateWindow(hWndMain);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
