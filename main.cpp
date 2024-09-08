#include <windows.h>
#include <D3dkmthk.h>
#include <chrono>
#include <memory>

using namespace std::chrono;

const char g_szClassName[] = "desktopBFIwindowClass";
bool quitProgram = false;
bool frameVisible;

std::unique_ptr<StrobeAPI> strobe;
char* strobeInfo = new char[4096];
bool debugMode = false;
int frameSnapshot = 0;

// Window event handling
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        if (strobe && debugMode && strobeInfo && strobe->frameCount(StrobeAPI::TotalFrame) - frameSnapshot >= 10)
        {
            RECT rec;
            SetRect(&rec, 10, 10, 500, 600);
            HBRUSH brush = CreateSolidBrush(RGB(255, 255, 255));
            FillRect(hdc, &rec, brush);
            snprintf(strobeInfo, 4096, "%s", strobe->getDebugInformation());
            DrawText(hdc, strobeInfo, strlen(strobeInfo), &rec, DT_TOP | DT_LEFT);
            frameSnapshot = strobe->frameCount(StrobeAPI::TotalFrame);
        }
        EndPaint(hwnd, &ps);
        ReleaseDC(hwnd, hdc);
        break;
    }
    case WM_CLOSE:
        quitProgram = true;
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        quitProgram = true;
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

bool CreateWindow(HINSTANCE hInstance, int nCmdShow)
{
    WNDCLASSEX wc;
    HWND hwnd;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = g_szClassName;
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc))
    {
        MessageBox(NULL, "Window Registration Failed!", "Error!",
            MB_ICONEXCLAMATION | MB_OK);
        return false;
    }

    hwnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST,
        g_szClassName,
        "DesktopBFI",
        WS_POPUP,
        0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
        NULL, NULL, hInstance, NULL);

    if (hwnd == NULL)
    {
        MessageBox(NULL, "Window Creation Failed!", "Error!",
            MB_ICONEXCLAMATION | MB_OK);
        return false;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    return true;
}

bool SetupVSync(HWND hwnd)
{
    D3DKMT_OPENADAPTERFROMHDC oa;
    oa.hDc = GetDC(hwnd);
    NTSTATUS result = D3DKMTOpenAdapterFromHdc(&oa);
    if (result != STATUS_SUCCESS) {
        // Handle error
        if (result == STATUS_INVALID_PARAMETER) {
            MessageBox(NULL, "D3DKMTOpenAdapterFromHdc function received an invalid parameter.", "Error!",
                MB_ICONEXCLAMATION | MB_OK);
        } else if (result == STATUS_NO_MEMORY) {
            MessageBox(NULL, "D3DKMTOpenAdapterFromHdc function, kernel ran out of memory.", "Error!",
                MB_ICONEXCLAMATION | MB_OK);
        }
        return false;
    }

    D3DKMT_WAITFORVERTICALBLANKEVENT we;
    we.hAdapter = oa.hAdapter;
    we.hDevice = 0;
    we.VidPnSourceId = oa.VidPnSourceId;

    return true;
}

int RunMainLoop(HWND hwnd)
{
    D3DKMT_WAITFORVERTICALBLANKEVENT we;
    D3DKMT_GETSCANLINE gsl;
    gsl.hAdapter = we.hAdapter;
    gsl.VidPnSourceId = we.VidPnSourceId;

    while (!quitProgram)
    {
        NTSTATUS result = D3DKMTWaitForVerticalBlankEvent(&we);
        if (result != STATUS_SUCCESS) {
            // Handle error
            break;
        }

        do {
            result = D3DKMTGetScanLine(&gsl);
        } while (gsl.InVerticalBlank == TRUE);

        if (strobe)
            frameVisible = strobe->strobe();
        else
            frameVisible = true;

        // Window transparency: 0 is invisible, 255 is opaque
        SetLayeredWindowAttributes(hwnd, 0, (frameVisible ? 0 : 1) * 255, LWA_ALPHA);
        RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);

        while (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE) > 0) {
            TranslateMessage(&Msg);
            DispatchMessage(&Msg);
        }
    }

    return Msg.wParam;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

    if (!CreateWindow(hInstance, nCmdShow)) {
        return 0;
    }

    HWND hwnd = FindWindowA(g_szClassName, "DesktopBFI");
    if (!hwnd) {
        MessageBox(NULL, "Window not found!", "Error!",
            MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    if (!SetupVSync(hwnd)) {
        return 0;
    }

    LPWSTR* szArglist;
    int argCount;
    szArglist = CommandLineToArgvW(GetCommandLineW(), &argCount);
    if (argCount == 4)
    {
        int sMode = _wtoi(szArglist);
        int pSInterval = _wtoi(szArglist);
        debugMode = _wtoi(szArglist);

        strobe = std::make_unique<StrobeCore>(sMode, pSInterval);
    }
    else
    {
        strobe = std::make_unique<StrobeCore>();
    }

    return RunMainLoop(hwnd);
}
