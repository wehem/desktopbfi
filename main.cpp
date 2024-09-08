#include <windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <chrono>
#include <wrl.h>
#include <thread>
#include <vector>

#include "strobe-api/strobe/strobe-core.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

using namespace Microsoft::WRL;
using namespace std::chrono;

const char g_szClassName[] = "desktopBFIwindowClass";
bool quitProgram = false;
bool frameVisible;

StrobeAPI* strobe = nullptr;
char* strobeInfo = (char*)malloc(sizeof(char) * 4096);
bool debugMode = false;
int frameSnapshot = 0;

// DirectX 12 objects
ComPtr<ID3D12Device> d3d12Device;
ComPtr<IDXGIFactory4> dxgiFactory;
ComPtr<IDXGIAdapter1> dxgiAdapter;
ComPtr<IDXGIOutput> dxgiOutput;
ComPtr<ID3D12CommandQueue> commandQueue;
ComPtr<ID3D12CommandAllocator> commandAllocator;
ComPtr<ID3D12GraphicsCommandList> commandList;
ComPtr<ID3D12Fence> fence;
HANDLE fenceEvent;
UINT64 fenceValue = 0;

// Swap chain objects
ComPtr<IDXGISwapChain3> swapChain;
std::vector<ComPtr<ID3D12Resource>> backBuffers;
std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> backBufferRTVHandles;
UINT backBufferIndex = 0;

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

   hr = D3D12CreateDevice(dxgiAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3d12Device));
   if (FAILED(hr))
   {
       MessageBox(NULL, "Failed to create D3D12 Device!", "Error!", MB_ICONEXCLAMATION | MB_OK);
       return 0;
   }

   // Create command queue
   D3D12_COMMAND_QUEUE_DESC queueDesc = {};
   queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
   queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

   hr = d3d12Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));
   if (FAILED(hr))
   {
       MessageBox(NULL, "Failed to create command queue!", "Error!", MB_ICONEXCLAMATION | MB_OK);
       return 0;
   }

   // Create command allocator and command list
   hr = d3d12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
   if (FAILED(hr))
   {
       MessageBox(NULL, "Failed to create command allocator!", "Error!", MB_ICONEXCLAMATION | MB_OK);
       return 0;
   }

   hr = d3d12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
   if (FAILED(hr))
   {
       MessageBox(NULL, "Failed to create command list!", "Error!", MB_ICONEXCLAMATION | MB_OK);
       return 0;
   }

   // Create synchronization objects
   hr = d3d12Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
   if (FAILED(hr))
{
    MessageBox(NULL, "Failed to create fence!", "Error!", MB_ICONEXCLAMATION | MB_OK);
    return 0;
}
fenceValue = 1;

fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
if (fenceEvent == nullptr)
{
    hr = HRESULT_FROM_WIN32(GetLastError());
    if (FAILED(hr))
    {
        MessageBox(NULL, "Failed to create fence event!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
}

// Create swap chain
DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
swapChainDesc.BufferCount = 2;
swapChainDesc.Width = GetSystemMetrics(SM_CXSCREEN);
swapChainDesc.Height = GetSystemMetrics(SM_CYSCREEN);
swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
swapChainDesc.SampleDesc.Count = 1;

ComPtr<IDXGISwapChain1> swapChain1;
hr = dxgiFactory->CreateSwapChainForHwnd(
    commandQueue.Get(),
    hwnd,
    &swapChainDesc,
    nullptr,
    nullptr,
    &swapChain1
);
if (FAILED(hr))
{
    MessageBox(NULL, "Failed to create swap chain!", "Error!", MB_ICONEXCLAMATION | MB_OK);
    return 0;
}

hr = swapChain1.As(&swapChain);
if (FAILED(hr))
{
    MessageBox(NULL, "Failed to query swap chain interface!", "Error!", MB_ICONEXCLAMATION | MB_OK);
    return 0;
}

// Create RTV descriptor heap
D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
rtvHeapDesc.NumDescriptors = swapChainDesc.BufferCount;
rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

ComPtr<ID3D12DescriptorHeap> rtvHeap;
hr = d3d12Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap));
if (FAILED(hr))
{
    MessageBox(NULL, "Failed to create RTV descriptor heap!", "Error!", MB_ICONEXCLAMATION | MB_OK);
    return 0;
}

// Create back buffer RTVs
D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
for (UINT i = 0; i < swapChainDesc.BufferCount; i++)
{
    ComPtr<ID3D12Resource> backBuffer;
    hr = swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer));
    if (FAILED(hr))
    {
        MessageBox(NULL, "Failed to get swap chain buffer!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    backBuffers.push_back(backBuffer);
    d3d12Device->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);
    backBufferRTVHandles.push_back(rtvHandle);
    rtvHandle.ptr += d3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
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
                // Busy-waiting for the poll time to elapse
            }
            DXGI_OUTPUT_DESC outputDesc;
            dxgiOutput->GetDesc(&outputDesc);
            inVBlank = outputDesc.DesktopCoordinates.bottom == 0; // Assuming VBlank when desktop coordinates are 0
        } while (inVBlank);

        // Record commands
        commandAllocator->Reset();
        commandList->Reset(commandAllocator.Get(), nullptr);

        // Transition the back buffer resource to the present state
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = backBuffers[backBufferIndex].Get();
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT; // Assuming it was in present state from previous frame
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        commandList->ResourceBarrier(1, &barrier);

        // If frame is not visible, clear the back buffer to black
        if (!frameVisible) {
            FLOAT clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f }; // Black color
			commandList->ClearRenderTargetView(backBufferRTVHandles[backBufferIndex], clearColor, 0, nullptr);
        }

        // Transition the back buffer resource back to the present state
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        commandList->ResourceBarrier(1, &barrier);

        commandList->Close();

        // Execute the command list
        ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
        commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

        // Present the frame (this is where you would integrate with your strobe logic)
        if (strobe)
            frameVisible = strobe->strobe();
        else
            frameVisible = true;

        // Window transparency: 0 is invisible, 255 is opaque
        SetLayeredWindowAttributes(hwnd, 0, (frameVisible ? 0 : 1) * 255, LWA_ALPHA);
        RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);

        // Present the frame
        hr = swapChain->Present(1, 0);
        if (FAILED(hr))
        {
            MessageBox(NULL, "Failed to present swap chain!", "Error!", MB_ICONEXCLAMATION | MB_OK);
            break;
        }

        // Signal the fence
        const UINT64 currentFenceValue = fenceValue;
        commandQueue->Signal(fence.Get(), currentFenceValue);
        fenceValue++;

        // Wait for the fence
        if (fence->GetCompletedValue() < currentFenceValue)
        {
            fence->SetEventOnCompletion(currentFenceValue, fenceEvent);
            WaitForSingleObject(fenceEvent, INFINITE);
        }

        // Get the next back buffer index
        backBufferIndex = swapChain->GetCurrentBackBufferIndex();

        // Handle window messages
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

    CloseHandle(fenceEvent);

    return (int)Msg.wParam;
}
