#include <windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <chrono>

#include "strobe-api/strobe/strobe-core.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

using namespace std::chrono;

const char g_szClassName[] = "desktopBFIwindowClass";
bool quitProgram = false;
bool frameVisible;

StrobeAPI* strobe = nullptr;
char* strobeInfo = (char*)malloc(sizeof(char) * 4096);
bool debugMode = false;
int frameSnapshot = 0;

ID3D12Device* d3d12Device = nullptr;
IDXGIFactory4* dxgiFactory = nullptr;
IDXGIAdapter1* dxgiAdapter = nullptr;
IDXGIOutput* dxgiOutput = nullptr;

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
            snprintf(strobeInfo, sizeof(char) * 4096, "%s", strobe->getDebugInformation());
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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

    WNDCLASSEX wc;
    HWND hwnd;
    MSG Msg;

    // Registering the Window Class
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
        return 0;
    }

    // Creating the Window
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
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    LPWSTR* szArglist;
    int argCount;
    szArglist = CommandLineToArgvW(GetCommandLineW(), &argCount);
    if (argCount == 4)
    {
        int sMode = _wtoi(szArglist[1]);
        int pSInterval = _wtoi(szArglist[2]);
        debugMode = _wtoi(szArglist[3]);

        strobe = new StrobeCore(sMode, pSInterval);
    }
    else
    {
        strobe = new StrobeCore();
    }

    // Initialize DirectX 12
    HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
    if (FAILED(hr))
    {
        MessageBox(NULL, "Failed to create DXGI Factory!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    hr = dxgiFactory->EnumAdapters1(0, &dxgiAdapter);
    if (FAILED(hr))
    {
        MessageBox(NULL, "Failed to enumerate DXGI Adapter!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    hr = dxgiAdapter->EnumOutputs(0, &dxgiOutput);
    if (FAILED(hr))
    {
        MessageBox(NULL, "Failed to enumerate DXGI Output!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    hr = D3D12CreateDevice(dxgiAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3d12Device));
    if (FAILED(hr))
    {
        MessageBox(NULL, "Failed to create D3D12 Device!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    while (!quitProgram)
    {
        // Wait for VBlank
        hr = dxgiOutput->WaitForVBlank();
        if (FAILED(hr))
        {
            MessageBox(NULL, "Failed to wait for VBlank!", "Error!", MB_ICONEXCLAMATION | MB_OK);
            break;
        }

        // Poll for VBlank exit
        BOOL inVBlank = TRUE;
        do {
            high_resolution_clock::time_point pollTime = high_resolution_clock::now() + microseconds(100);
            while (pollTime > high_resolution_clock::now())
            {
            }
            DXGI_OUTPUT_DESC outputDesc;
            dxgiOutput->GetDesc(&outputDesc);
            inVBlank = outputDesc.DesktopCoordinates.bottom == 0; // Assuming VBlank when desktop coordinates are 0
        } while (inVBlank);

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

    // Cleanup
    if (strobeInfo)
    {
        free(strobeInfo);
        strobeInfo = nullptr;
    }

    if (strobe)
    {
        delete strobe;
        strobe = nullptr;
    }

    if (dxgiOutput)
    {
        dxgiOutput->Release();
        dxgiOutput = nullptr;
    }

    if (dxgiAdapter)
    {
        dxgiAdapter->Release();
        dxgiAdapter = nullptr;
    }

    if (dxgiFactory)
    {
        dxgiFactory->Release();
        dxgiFactory = nullptr;
    }

    if (d3d12Device)
    {
        d3d12Device->Release();
        d3d12Device = nullptr;
    }

    return (int)Msg.wParam;
}
