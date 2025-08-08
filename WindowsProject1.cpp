#include <windows.h>
#include <commdlg.h>
#include <tchar.h>
#include <commctrl.h> // 头部添加
#include <ctime> // 头部添加
#include <cstdlib> // 头部添加
#pragma comment(lib, "comctl32.lib")

#define IDC_BTN_UPLOAD 2001

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
{
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = _T("LeziWindow");
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&wc);

    HWND hWnd = CreateWindow(_T("LeziWindow"), _T("乐子哥判定"),
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, // 去掉最大化按钮
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 250,
        nullptr, nullptr, hInstance, nullptr);

    // 加载图标
    HICON hIcon = (HICON)LoadImage(NULL,_T(""), IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);
    if (hIcon)
    {
        SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
        SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}

void ShowJudgingDialog(HWND hParent)
{
    // 创建一个顶层窗口作为进度条弹窗
    HWND hDlg = CreateWindowEx(WS_EX_TOPMOST, _T("STATIC"), _T(""),
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, 350, 120,
        hParent, NULL, GetModuleHandle(NULL), NULL);

    // 居中弹窗
    RECT rcParent;
    GetWindowRect(hParent, &rcParent);
    int x = rcParent.left + (rcParent.right - rcParent.left - 350) / 2;
    int y = rcParent.top + (rcParent.bottom - rcParent.top - 120) / 2;
    SetWindowPos(hDlg, HWND_TOP, x, y, 350, 120, SWP_SHOWWINDOW);

    // 创建进度条
    HWND hProgress = CreateWindowEx(0, PROGRESS_CLASS, NULL,
        WS_CHILD | WS_VISIBLE,
        30, 50, 280, 30,
        hDlg, NULL, GetModuleHandle(NULL), NULL);

    SendMessage(hProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
    SendMessage(hProgress, PBM_SETPOS, 0, 0);

    // 显示弹窗
    ShowWindow(hDlg, SW_SHOW);
    UpdateWindow(hDlg);

    // 居中显示“正在判断中，请稍等...”文本
    HDC hdc = GetDC(hDlg);
    RECT textRect = { 0, 10, 350, 40 };
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(0, 0, 0));
    DrawText(hdc, _T("正在判断中，请稍等..."), -1, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    ReleaseDC(hDlg, hdc);

    // 进度条动画
    for (int i = 0; i <= 100; ++i)
    {
        SendMessage(hProgress, PBM_SETPOS, i, 0);
        Sleep(6000); // 50ms * 100 = 5秒
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT) break;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    DestroyWindow(hDlg);
}

// 新增弹窗窗口过程
LRESULT CALLBACK LeziMsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        RECT rect;
        GetClientRect(hWnd, &rect);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(255, 0, 0));
        DrawText(hdc, _T("警告，此人是乐子哥"), -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        EndPaint(hWnd, &ps);
    }
        break;
    case WM_CLOSE:
        DestroyWindow(hWnd);
        break;
    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

void ShowLeziWarning(HWND hWnd)
{
    RECT rcScreen;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &rcScreen, 0);
    int width = 300, height = 120;
    srand((unsigned int)time(nullptr));

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = LeziMsgProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = _T("LeziMsgClass");
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&wc);

    for (int i = 0; i < 50; ++i)
    {
        int x = rcScreen.left + rand() % (rcScreen.right - rcScreen.left - width);
        int y = rcScreen.top + rand() % (rcScreen.bottom - rcScreen.top - height);

        HWND hMsg = CreateWindowEx(WS_EX_TOPMOST, _T("LeziMsgClass"), _T("警告"),
            WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_SYSMENU,
            x, y, width, height,
            hWnd, NULL, GetModuleHandle(NULL), NULL);

        ShowWindow(hMsg, SW_SHOW);
        UpdateWindow(hMsg);

        MessageBeep(MB_ICONWARNING); // 播放提示音
        Sleep(80);
    }
}

// 在 ShowOpenFileDialog 判断为乐子哥时调用
void ShowOpenFileDialog(HWND hWnd)
{
    OPENFILENAME ofn = { 0 };
    TCHAR szFile[MAX_PATH] = { 0 };
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFilter = _T("图片文件 (*.bmp;*.jpg;*.jpeg;*.png)\0*.bmp;*.jpg;*.jpeg;*.png\0所有文件 (*.*)\0*.*\0");
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    ofn.lpstrTitle = _T("请选择一张照片");

    if (GetOpenFileName(&ofn))
    {
        ShowJudgingDialog(hWnd);
        // 假设此处判断为乐子哥
        ShowLeziWarning(hWnd);
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        CreateWindow(_T("BUTTON"), _T("上传照片"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            150, 150, 100, 30, hWnd, (HMENU)IDC_BTN_UPLOAD, nullptr, nullptr);
        break;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_BTN_UPLOAD)
        {
            ShowOpenFileDialog(hWnd);
        }
        break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // 居中显示主标题
        const TCHAR* text = _T("判断你是不是乐子哥");
        RECT rect;
        GetClientRect(hWnd, &rect);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(0, 0, 0));
        DrawText(hdc, text, -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

        // 在窗口底部居中显示作者信息
        const TCHAR* author = _T("作者：余生军徒儿—K7rift");
        RECT authorRect = rect;
        authorRect.top = rect.bottom - 30; // 距底部约40像素
        authorRect.bottom = rect.bottom - 10; // 距底部约10像素
        DrawText(hdc, author, -1, &authorRect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

        EndPaint(hWnd, &ps);
    }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}