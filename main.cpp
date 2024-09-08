#include <windows.h>
#include <dxgi1_3.h>
#include <d3d11.h>
#include <chrono>
#include <memory>
#include <string>
#include "strobe-api/strobe/strobe-core.h"

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

using namespace std::chrono;

const char g_szClassName[] = "desktopBFIwindowClass";
bool quitProgram = false;
bool frameVisible = true;

std::unique_ptr<StrobeAPI> strobe;
std::string strobeInfo;
bool debugMode = false;
int frameSnapshot = 0;

IDXGIOutput* dxgiOutput = nullptr;
IDXGISwapChain1* swapChain = nullptr;

// Window event handling
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        if (strobe && debugMode && !strobeInfo.empty() && strobe->frameCount(StrobeAPI::TotalFrame) - frameSnapshot >= 10)
        {
            RECT rec;
            SetRect(&rec, 10, 10, 500, 600);
            HBRUSH brush = CreateSolidBrush(RGB(255, 255, 255));
            FillRect(hdc, &rec, brush);
            DeleteObject(brush);
            DrawTextA(hdc, strobeInfo.c_str(), -1, &rec, DT_TOP | DT_LEFT);
            frameSnapshot = strobe->frameCount(StrobeAPI::TotalFrame);
        }
        EndPaint(hwnd, &ps);
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

bool CreateMainWindow(HINSTANCE hInstance, int nCmdShow, HWND& hwnd)
{
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), 0, WndProc, 0, 0, hInstance,
                      LoadIcon(NULL, IDI_APPLICATION), LoadCursor(NULL, IDC_ARROW),
                      (HBRUSH)GetStockObject(BLACK_BRUSH), NULL, g_szClassName,
                      LoadIcon(NULL, IDI_APPLICATION) };

    if (!RegisterClassEx(&wc))
    {
        MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return false;
    }

    hwnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST,
        g_szClassName, "DesktopBFI", WS_POPUP,
        0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
        NULL, NULL, hInstance, NULL);

    if (hwnd == NULL)
    {
        MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return false;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    return true;
}

bool SetupDXGI(HWND hwnd)
{
    IDXGIFactory2* factory = nullptr;
    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* context = nullptr;

    HRESULT hr = CreateDXGIFactory2(0, __uuidof(IDXGIFactory2), (void**)&factory);
    if (FAILED(hr))
    {
        MessageBox(NULL, "Failed to create DXGI Factory!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return false;
    }

    hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0,
                           D3D11_SDK_VERSION, &device, nullptr, &context);
    if (FAILED(hr))
    {
        MessageBox(NULL, "Failed to create D3D11 Device!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        factory->Release();
        return false;
    }

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = 2;
    swapChainDesc.Width = 0;
    swapChainDesc.Height = 0;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    swapChainDesc.SampleDesc.Count = 1;

    hr = factory->CreateSwapChainForHwnd(device, hwnd, &swapChainDesc, nullptr, nullptr, &swapChain);
    if (FAILED(hr))
    {
        MessageBox(NULL, "Failed to create Swap Chain!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        device->Release();
        context->Release();
        factory->Release();
        return false;
    }

    IDXGIAdapter* adapter = nullptr;
    hr = factory->EnumAdapters(0, &adapter);
    if (FAILED(hr))
    {
        MessageBox(NULL, "Failed to enumerate adapters!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        swapChain->Release();
        device->Release();
        context->Release();
        factory->Release();
        return false;
    }

    hr = adapter->EnumOutputs(0, &dxgiOutput);
    if (FAILED(hr))
    {
        MessageBox(NULL, "Failed to enumerate outputs!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        adapter->Release();
        swapChain->Release();
        device->Release();
        context->Release();
        factory->Release();
        return false;
    }

    adapter->Release();
    device->Release();
    context->Release();
    factory->Release();

    return true;
}

int RunMainLoop(HWND hwnd)
{
    MSG msg;
    DXGI_OUTPUT_DESC outputDesc;
    dxgiOutput->GetDesc(&outputDesc);

    while (!quitProgram)
    {
        dxgiOutput->WaitForVBlank();

        if (strobe)
            frameVisible = strobe->strobe();
        else
            frameVisible = true;

        // Window transparency: 0 is invisible, 255 is opaque
        SetLayeredWindowAttributes(hwnd, 0, (frameVisible ? 0 : 1) * 255, LWA_ALPHA);
        RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);

        if (debugMode && strobe)
        {
            strobeInfo = strobe->getDebugInformation();
        }

        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) > 0)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        swapChain->Present(1, 0); // VSync is on
    }

    return (int)msg.wParam;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

    HWND hwnd;
    if (!CreateMainWindow(hInstance, nCmdShow, hwnd)) {
        return 0;
    }

    if (!SetupDXGI(hwnd)) {
        return 0;
    }

    LPWSTR* szArglist;
    int argCount;
    szArglist = CommandLineToArgvW(GetCommandLineW(), &argCount);
    if (szArglist == NULL)
    {
        MessageBox(NULL, "Unable to parse command line!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    if (argCount == 4)
    {
        int sMode = _wtoi(szArglist[1]);
        int pSInterval = _wtoi(szArglist[2]);
        debugMode = _wtoi(szArglist[3]) != 0;

        try {
            strobe = std::make_unique<StrobeCore>(sMode, pSInterval);
        }
        catch (const std::exception& e) {
            MessageBox(NULL, e.what(), "Error!", MB_ICONEXCLAMATION | MB_OK);
            LocalFree(szArglist);
            return 0;
        }
    }
    else
    {
        try {
            strobe = std::make_unique<StrobeCore>();
        }
        catch (const std::exception& e) {
            MessageBox(NULL, e.what(), "Error!", MB_ICONEXCLAMATION | MB_OK);
            LocalFree(szArglist);
            return 0;
        }
    }

    LocalFree(szArglist);

    int result = RunMainLoop(hwnd);

    // Clean up
    if (dxgiOutput) dxgiOutput->Release();
    if (swapChain) swapChain->Release();

    return result;
}
