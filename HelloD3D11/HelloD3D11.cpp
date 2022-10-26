// HelloD3D11.cpp : Defines the entry point for the application.
//
#include "framework.h"
#include "HelloD3D11.h"
#include "resource.h"

#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include "directxtex.h"
#include <initguid.h>
#include "IImageWrapper.h" 
struct ImageFrame {
    BYTE* pBuffer;
    UINT unWidth;
    UINT unHeight;
    UINT unStride;
};

using namespace DirectX;
//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------
struct SimpleVertex
{
    XMFLOAT3 Pos;
    XMFLOAT3 Normal;
    XMFLOAT2 Tex;
};

struct ConstantBuffer
{
    XMMATRIX mWorld;
    XMMATRIX mView;
    XMMATRIX mProjection;
    XMFLOAT4 vLightDir[2];
    XMFLOAT4 vLightColor[2];
    XMFLOAT4 vOutputColor;
};

// Cubemap
struct CubeVertex
{
    XMFLOAT3 Pos;
};

struct ConstCubeBuffer
{
    XMMATRIX mWorld;
    XMMATRIX mView;
    XMMATRIX mProjection;
};
//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
#define MAX_LOADSTRING 100
#define SCREEN_WIDTH   800
#define SCREEN_HEIGHT  600

HINSTANCE                   g_hInst = NULL;
HWND                        g_hWnd = NULL;
UINT                        g_width = SCREEN_WIDTH;
UINT                        g_height = SCREEN_HEIGHT;

XMMATRIX                    g_View;
XMMATRIX                    g_Projection;

D3D_DRIVER_TYPE             g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL           g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device*               g_pd3dDevice = NULL;
ID3D11DeviceContext*        g_pImmediateContext = NULL;
IDXGISwapChain*             g_pSwapChain = NULL;
ID3D11RenderTargetView*     g_pRenderTargetView = NULL;
ID3D11RasterizerState*      g_pRasterizerState = NULL;
ID3D11Texture2D*            g_pDepthStencil = NULL;
ID3D11DepthStencilView*     g_pDepthStencilView = NULL;
ID3D11VertexShader*         g_pVertexShader = NULL;
ID3DBlob*                   g_pVSBlob = NULL;
ID3D11PixelShader*          g_pPixelShader = NULL;
ID3D11PixelShader*          g_pPSSolid = NULL;
ID3D11InputLayout*          g_pVertexLayout = NULL;
ID3D11Buffer*               g_pVertexBuffer = NULL;
ID3D11Buffer*               g_pIndexBuffer = NULL;
ID3D11Buffer*               g_pConstantBuffer = NULL;
ID3D11ShaderResourceView*   g_pTextureRV = NULL;    // for uv texture 
ID3D11SamplerState*         g_pSamplerLinear = NULL;

//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

ATOM                MyRegisterClass(HINSTANCE);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
HRESULT             InitDevice(UINT, UINT);
HRESULT             CreateRenderTargetView(UINT, UINT);
HRESULT             CreateRasterState();
HRESULT             CreateDepthBuffer(UINT, UINT);
void                SetViewPort(UINT, UINT);
HRESULT             InitShaders();
HRESULT             InitVertex();
HRESULT             InitConstBuffer(UINT, UINT);
HRESULT             CreateSamplerLinear();
HRESULT             LoadTexture();
void                CleanupDevice();
void                Render();

// IBL
HRESULT             InitIBL();
HRESULT             InitSkyboxShaders();
void                CleanupIBL();


// IBL, cubmap, skybox
ID3DBlob*                   g_pCubeVSBlob = NULL;
ID3D11VertexShader*         g_pCubeVertexShader = NULL;
ID3D11PixelShader*          g_pCubePixelShader = NULL;
ID3D11InputLayout*          g_pCubeVertexLayout = NULL;
ID3D11Buffer*               g_pCubeVertexBuffer = NULL;
ID3D11Buffer*               g_pCubeIndexBuffer = NULL;
ID3D11Buffer*               g_pCubeConstBuffer = NULL;
ID3D11Texture2D*            g_pCubeTex = NULL;
ID3D11ShaderResourceView*   g_pCubeTexSR = NULL;
ID3D11RenderTargetView*     g_pCubeMapRTVs[6] = { NULL };
ID3D11DepthStencilView*     g_pCubeMapDSV = NULL;

ID3D11ShaderResourceView*   g_pHDRTextureRV = NULL;    // for hdr texture 
XMMATRIX                    g_CubeViews[6];
XMMATRIX                    g_CubeProjection;

ID3D11VertexShader*         g_pSkyboxVertexShader = NULL;
ID3D11PixelShader*          g_pSkyboxPixelShader = NULL;
ID3D11RasterizerState*      g_pSkyboxRasterizerState = NULL;
ID3D11DepthStencilState*    g_pSkyboxDepthState = NULL;


HRESULT InitHDRRenderTarget(UINT);
HRESULT InitIBLShaders();
HRESULT InitCubeVertex();
HRESULT LoadHDRTexture();
HRESULT InitIBLConstBuffer();
HRESULT CreateSkyboxRasterState();
void    DrawCubeMap(UINT);


//------------------------------------------------------------------------------------
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    HRESULT hr = CoInitialize(NULL);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_HELLOD3D11, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_HELLOD3D11));

    RECT rc;
    GetClientRect(g_hWnd, &rc);
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    hr = InitDevice(width, height);
    if (FAILED(hr))
    {
        CleanupDevice();
        return 0;
    }

    // Main message loop
    MSG msg = { 0 };
    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            Render();
        }
    }
    return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_HELLOD3D11));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_HELLOD3D11);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable
   RECT rc = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
   HWND hWnd = CreateWindowW(szWindowClass, 
                             szTitle, 
                             WS_OVERLAPPEDWINDOW,
                             100,
                             50,   
                             rc.right - rc.left, 
                             rc.bottom - rc.top, 
                             nullptr, 
                             nullptr, 
                             hInstance, 
                             nullptr);
   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   g_hInst = hInstance;
   g_hWnd = hWnd;

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        CleanupDevice();
        break;
    case WM_SIZE:
        //TODO: re-initialize the device? change viewport, projection
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

HRESULT CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    hr = D3DCompileFromFile(szFileName, 
                            NULL, 
                            NULL, 
                            szEntryPoint, 
                            szShaderModel,
                            dwShaderFlags, 
                            NULL, 
                            ppBlobOut, 
                            &pErrorBlob);
    if (FAILED(hr))
    {
        if (pErrorBlob != NULL)
            OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
        if (pErrorBlob) pErrorBlob->Release();
        return hr;
    }
    if (pErrorBlob) pErrorBlob->Release();

    return hr;
}

HRESULT InitDevice(UINT width, UINT height)
{
    g_width = width;
    g_height = height;

    HRESULT hr = S_OK;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = g_hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
    {
        g_driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDeviceAndSwapChain( NULL,
                                            g_driverType,
                                            NULL,
                                            createDeviceFlags,
                                            featureLevels,
                                            numFeatureLevels,
                                            D3D11_SDK_VERSION,
                                            &sd,
                                            &g_pSwapChain,
                                            &g_pd3dDevice,
                                            &g_featureLevel,
                                            &g_pImmediateContext);
        if (SUCCEEDED(hr))
            break;
    }
    if (FAILED(hr))
        return hr;

    //(1) load HDR and convert to cubemap as skybox's resource views (6) 
    hr = InitIBL();
    if (FAILED(hr))
        return hr;

    //(2) init shader objects for drawing skybox
    hr = InitSkyboxShaders();
    if (FAILED(hr))
        return hr;

    //(3) init world's rendering objects
    hr |= InitShaders();
    hr |= InitVertex();
    hr |= LoadTexture();
    hr |= InitConstBuffer(width, height);
    hr |= CreateRenderTargetView(width, height);
    hr |= CreateDepthBuffer(width, height);
    hr |= CreateRasterState();

    return hr;
}

HRESULT CreateRenderTargetView(UINT width, UINT height)
{
    HRESULT hr = S_OK;
    // Create a render target view
    ID3D11Texture2D* pBackBuffer = NULL;
    hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
    if (FAILED(hr))
        return hr;

    hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_pRenderTargetView);
    pBackBuffer->Release();
    return hr;
}

HRESULT CreateRasterState()
{
    D3D11_RASTERIZER_DESC RSState;
    RSState.FillMode = D3D11_FILL_SOLID;
    //RSState.FillMode = D3D11_FILL_WIREFRAME;
    //RSState.CullMode = D3D11_CULL_BACK;
    RSState.CullMode = D3D11_CULL_NONE;
    RSState.FrontCounterClockwise = TRUE;
    RSState.DepthBias = 0;
    RSState.DepthBiasClamp = 0.0f;
    RSState.SlopeScaledDepthBias = 0.0f;
    RSState.DepthClipEnable = FALSE;
    RSState.ScissorEnable = FALSE;
    RSState.MultisampleEnable = TRUE;
    RSState.AntialiasedLineEnable = TRUE;

    HRESULT hr = g_pd3dDevice->CreateRasterizerState(&RSState, &g_pRasterizerState);
    return hr;
}

HRESULT CreateDepthBuffer(UINT width, UINT height)
{
    HRESULT hr = S_OK;

    // Create depth stencil texture
    D3D11_TEXTURE2D_DESC descDepth;
    ZeroMemory(&descDepth, sizeof(descDepth));
    descDepth.Width = width;
    descDepth.Height = height;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;   // depth 24bits, stencil 8bits
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;
    hr = g_pd3dDevice->CreateTexture2D(&descDepth, NULL, &g_pDepthStencil);
    if (FAILED(hr))
        return hr;

    // Create the depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
    ZeroMemory(&descDSV, sizeof(descDSV));
    descDSV.Format = descDepth.Format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;
    hr = g_pd3dDevice->CreateDepthStencilView(g_pDepthStencil, &descDSV, &g_pDepthStencilView);

    return hr;
}

void SetViewPort(UINT width, UINT height)
{
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)width;
    vp.Height = (FLOAT)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;

    g_pImmediateContext->RSSetViewports(1, &vp);
}

HRESULT InitShaders()
{
    HRESULT hr = S_OK;

    // Compile the vertex shader
    hr = CompileShaderFromFile(L"shaders.fx", "VS", "vs_4_0", &g_pVSBlob);
    if (FAILED(hr))
    {
        MessageBox(NULL,
            L"The VS FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    // Create the vertex shader
    hr = g_pd3dDevice->CreateVertexShader(g_pVSBlob->GetBufferPointer(),
                                          g_pVSBlob->GetBufferSize(),
                                          NULL,
                                          &g_pVertexShader);
    if (FAILED(hr))
    {
        g_pVSBlob->Release();
        return hr;
    }

    // Compile the pixel shader (PS)
    ID3DBlob* pPSBlob = NULL;
    hr = CompileShaderFromFile(L"shaders.fx", "PS", "ps_4_0", &pPSBlob);
    if (FAILED(hr))
    {
        MessageBox(NULL,
            L"The PS FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    // Create the pixel shader (PS)
    hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(),
                                         pPSBlob->GetBufferSize(),
                                         NULL,
                                         &g_pPixelShader);
    pPSBlob->Release();
    if (FAILED(hr))
        return hr;

    // Compile the pixel shader (PSSolid)
    hr = CompileShaderFromFile(L"shaders.fx", "PSSolid", "ps_4_0", &pPSBlob);
    if (FAILED(hr))
    {
        MessageBox(NULL,
            L"The PS FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    // Create the pixel shader (PSSolid)
    hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(),
                                         pPSBlob->GetBufferSize(),
                                         NULL,
                                         &g_pPSSolid);
    pPSBlob->Release();
    if (FAILED(hr))
        return hr;

    return hr;
}

HRESULT InitVertex()
{
    HRESULT hr = S_OK;

    // Define the input layout (VAO)
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    UINT numElements = ARRAYSIZE(layout);

    // Create the input layout
    hr = g_pd3dDevice->CreateInputLayout(layout,
                                         numElements,
                                         g_pVSBlob->GetBufferPointer(),
                                         g_pVSBlob->GetBufferSize(),
                                         &g_pVertexLayout);
    g_pVSBlob->Release();
    if (FAILED(hr))
        return hr;

    // Set the input layout 
    //g_pImmediateContext->IASetInputLayout(g_pVertexLayout);


    // Create vertex buffer (VBO)
    SimpleVertex vertices[] =
    {
        // up
        { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3( 1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3( 1.0f, 1.0f,  1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(-1.0f, 1.0f,  1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f)  },
        // down
        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3( 1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 0.0) },
        { XMFLOAT3( 1.0f, -1.0f,  1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
        // left
        { XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
        // right
        { XMFLOAT3(1.0f, -1.0f,  1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(1.0f,  1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(1.0f,  1.0f,  1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
        // rear
        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3( 1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3( 1.0f,  1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
        // front
        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3( 1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3( 1.0f,  1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(-1.0f,  1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
    };
    UINT numVertexElements = ARRAYSIZE(vertices);

    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SimpleVertex) * numVertexElements;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = vertices;
    hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer);
    if (FAILED(hr))
        return hr;

    // Set vertex buffer
    //UINT stride = sizeof(SimpleVertex);
    //UINT offset = 0;
    //g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

    // Create index buffer, 6 faces
    WORD indices[] =
    {   // up
        3,1,0,2,1,3,
        // down
        6,4,5,7,4,6,
        // left
        11,9,8,10,9,11,
        // right
        14,12,13,15,12,14,
        // rear
        19,17,16,18,17,19,
        // front
        22,20,21,23,20,22
    };
    UINT numIndices = ARRAYSIZE(indices);

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(WORD) * numIndices;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;

    bd.CPUAccessFlags = 0;
    InitData.pSysMem = indices;
    hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pIndexBuffer);

    return hr;
}

HRESULT CreateSamplerLinear()
{
    HRESULT hr = S_OK;
    if (g_pSamplerLinear == NULL) {
        // Create the sample state
        D3D11_SAMPLER_DESC sampDesc;
        ZeroMemory(&sampDesc, sizeof(sampDesc));
        sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        sampDesc.MinLOD = 0;
        sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
        hr = g_pd3dDevice->CreateSamplerState(&sampDesc, &g_pSamplerLinear);
    }
    return hr;
}

HRESULT LoadTexture()
{
    HRESULT hr = S_OK;

    // Load the Texture
    DirectX::TexMetadata md;
    DirectX::ScratchImage img;
    hr = LoadFromDDSFile(L"seafloor.dds",
                         DDS_FLAGS_NONE, 
                         &md, 
                         img);
    if (FAILED(hr))
        return hr;
    
    hr = CreateShaderResourceView(g_pd3dDevice,
                                  img.GetImages(),
                                  img.GetImageCount(),
                                  md,
                                  &g_pTextureRV);
    if (FAILED(hr))
        return hr;

    hr = CreateSamplerLinear();    
    return hr;
}

HRESULT InitConstBuffer(UINT width, UINT height)
{
    HRESULT hr = S_OK;

    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(ConstantBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = g_pd3dDevice->CreateBuffer(&bd, NULL, &g_pConstantBuffer);
    if (FAILED(hr))
        return hr;

    // Initialize the camera view matrix
    XMVECTOR Eye = XMVectorSet(0.f, 1.f, -5.f, 0.f);
    XMVECTOR At  = XMVectorSet(0.f, 0.f,  0.f, 0.f);
    XMVECTOR Up  = XMVectorSet(0.f, 1.f,  0.f, 0.f);
    g_View = XMMatrixLookAtLH(Eye, At, Up);

    // Initialize the projection matrix
    g_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, (FLOAT)width / (FLOAT)height, 0.01f, 100.f);

    return S_OK;
}

void RenderWorld() 
{
    UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;
    g_pImmediateContext->IASetInputLayout(g_pVertexLayout);
    g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);
    g_pImmediateContext->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    // Update time
    static float t = 0.0f;
    if (g_driverType == D3D_DRIVER_TYPE_REFERENCE) {
        t += (float)XM_PI * 0.0125f;
    }
    else {
        static DWORD dwTimeStart = 0;
        DWORD dwTimeCur = (DWORD)GetTickCount64();
        if (dwTimeStart == 0)
            dwTimeStart = dwTimeCur;
        t = (dwTimeCur - dwTimeStart) / 1000.0f;
    }

    // Setup our lighting parameters
    XMFLOAT4 vLightDirs[2] =
    {
        XMFLOAT4(-0.3f, 0.3f, -0.5f, 1.0f),
        XMFLOAT4(0.0f, 0.0f, -0.8f, 1.0f),
    };
    XMFLOAT4 vLightColors[2] =
    {
        XMFLOAT4(0.8f, 0.8f, 0.3f, 1.0f),
        XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f)
    };

    // Rotate the second light around the origin
    XMMATRIX mRotate = XMMatrixRotationY(-1.0f * t);
    XMVECTOR vLightDir = XMLoadFloat4(&vLightDirs[1]);
    vLightDir = XMVector3Transform(vLightDir, mRotate);
    XMStoreFloat4(&vLightDirs[1], vLightDir);

    XMMATRIX world;
    world = XMMatrixRotationX(t) * XMMatrixRotationY(t);

    // update color 
    vLightColors[1].x = (sinf(t * 1.0f) + 1.0f) * 0.5f;
    vLightColors[1].y = (cosf(t * 3.0f) + 1.0f) * 0.5f;
    vLightColors[1].z = (sinf(t * 5.0f) + 1.0f) * 0.5f;

    ConstantBuffer cb1;
    cb1.mWorld = XMMatrixTranspose(world);
    cb1.mView = XMMatrixTranspose(g_View);
    cb1.mProjection = XMMatrixTranspose(g_Projection);
    cb1.vLightDir[0] = vLightDirs[0];
    cb1.vLightDir[1] = vLightDirs[1];
    cb1.vLightColor[0] = vLightColors[0];
    cb1.vLightColor[1] = vLightColors[1];
    cb1.vOutputColor = XMFLOAT4(0, 0, 0, 0);
    g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, NULL, &cb1, 0, 0);

    g_pImmediateContext->VSSetShader(g_pVertexShader, NULL, 0);
    g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
    g_pImmediateContext->PSSetShader(g_pPixelShader, NULL, 0);

    g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantBuffer);

    g_pImmediateContext->RSSetState(g_pRasterizerState);

    g_pImmediateContext->PSSetShaderResources(0, 1, &g_pTextureRV);

    g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);

    g_pImmediateContext->DrawIndexed(36, 0, 0);

    // Render each light
    for (int m = 0; m < 2; m++)
    {
        XMMATRIX mLight = XMMatrixTranslationFromVector(5.0f * XMLoadFloat4(&vLightDirs[m]));
        XMMATRIX mLightScale = XMMatrixScaling(0.05f, 0.1f, 0.1f);
        mLight = mLightScale * mLight;

        // Update the world variable to reflect the current light
        cb1.mWorld = XMMatrixTranspose(mLight);
        cb1.vOutputColor = vLightColors[m];
        g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, NULL, &cb1, 0, 0);

        g_pImmediateContext->PSSetShader(g_pPSSolid, NULL, 0);
        g_pImmediateContext->DrawIndexed(36, 0, 0);
    }
}

void RenderSkybox();
void Render()
{
    SetViewPort(g_width, g_height);
    g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);
    
    // Clear the back buffer &  depth buffer
    float ClearColor[4] = { 0.0f, 0.1f, 0.3f, 1.0f }; // red,green,blue,alpha
    g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, ClearColor);
    g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    RenderSkybox();
    RenderWorld();

    // Present our back buffer to our front buffer
    g_pSwapChain->Present(0, 0);
}

void CleanupDevice()
{
    if (g_pImmediateContext) g_pImmediateContext->ClearState();

    if (g_pSamplerLinear) g_pSamplerLinear->Release();
    if (g_pTextureRV) g_pTextureRV->Release();
    if (g_pConstantBuffer) g_pConstantBuffer->Release();
    if (g_pVertexBuffer) g_pVertexBuffer->Release();
    if (g_pIndexBuffer) g_pIndexBuffer->Release();
    if (g_pVertexLayout) g_pVertexLayout->Release();
    if (g_pVertexShader) g_pVertexShader->Release();
    if (g_pPixelShader) g_pPixelShader->Release();
    if (g_pRasterizerState) g_pRasterizerState->Release();
    if (g_pDepthStencil) g_pDepthStencil->Release();
    if (g_pDepthStencilView) g_pDepthStencilView->Release();
    if (g_pRenderTargetView) g_pRenderTargetView->Release();
    if (g_pSwapChain) g_pSwapChain->Release();
    if (g_pImmediateContext) g_pImmediateContext->Release();
    if (g_pd3dDevice) g_pd3dDevice->Release();

    CleanupIBL();

    CoUninitialize();
}

HRESULT LoadSkyboxImages();
HRESULT InitIBL() 
{
    HRESULT hr = S_OK;

    //step1: load HDR and convert to cubemap
    hr |= InitIBLShaders();
    hr |= InitCubeVertex();
    hr |= LoadHDRTexture();
    hr |= InitIBLConstBuffer();
    hr |= CreateSkyboxRasterState();

    if (FAILED(hr))
        return hr;

    UINT cubemapSize = 512;
    if (cubemapSize == 2048) {
        //test code
        hr = LoadSkyboxImages();
    } else {
        hr = InitHDRRenderTarget(cubemapSize);
        if (SUCCEEDED(hr)) {
            DrawCubeMap(cubemapSize);
        }
    }

    return hr;
}

void ReleaseImageBuffer(ImageFrame& currentImageFrame)
{
    if (currentImageFrame.pBuffer)
        delete[] currentImageFrame.pBuffer;
    currentImageFrame.pBuffer = NULL;
    currentImageFrame.unWidth = 0;
    currentImageFrame.unHeight = 0;
    currentImageFrame.unStride = 0;
}

BOOL LoadImageFile(const wchar_t* pszFilePath, ImageFrame& newImageFrame)
{
    IImageWrapper* pImageWrapper = NULL;
    HRESULT hr = CoCreateInstance(CLSID_IMAGE_WRAPPER, NULL, CLSCTX_INPROC_SERVER, IID_IMAGE_WRAPER, (void**)&pImageWrapper);
    if (FAILED(hr))
        return FALSE;

    if (pImageWrapper) {
        // enable AutoRotate.
        IImageWrapperConfig* pConfig = NULL;
        BOOL bAutoRotate = TRUE;
        hr = pImageWrapper->QueryInterface(IID_IMAGE_WRAPERCONFIG, (void**)&pConfig);
        if (pConfig)
        {
            hr = pConfig->SetConfig(IW_MODE_EnableAutoRotateByEXIF, &bAutoRotate);

            // set Display mode.
            IMAGEWRAPPER_DISPLAYMODE uMode = DM_LEFTONLY;
            hr = pConfig->SetConfig(IW_MODE_DisplayMode, &uMode);

            pConfig->Release();
            pConfig = NULL;
        }
        //OutputDebugStringA("To get image Buffer\r\n");

        hr = pImageWrapper->LoadImage((WCHAR*)pszFilePath);
        //OutputDebugStringA("After get image Buffer\r\n");

        UINT imgWidth = 0;
        UINT imgHeight = 0;
        pImageWrapper->GetImageWidth(&imgWidth);
        pImageWrapper->GetImageHeight(&imgHeight);

        /*char szBuffer[_MAX_PATH];
        sprintf_s(szBuffer, "Now get image %d, %d\r\n", imgWidth, imgHeight);
        OutputDebugStringA(szBuffer);*/
        if (SUCCEEDED(hr) && imgWidth != 0 && imgHeight != 0)
        {
            size_t bufsize = (size_t)imgWidth * imgWidth * 4;
            newImageFrame.unWidth = imgWidth;
            newImageFrame.unHeight = imgHeight;
            newImageFrame.unStride = imgWidth * 4;
            newImageFrame.pBuffer = new BYTE[bufsize];
            hr = pImageWrapper->GetFrame(newImageFrame.pBuffer, imgWidth, imgHeight, newImageFrame.unStride);
            if (FAILED(hr))
            {
                delete[] newImageFrame.pBuffer;
                newImageFrame.pBuffer = NULL;
            }

        }
        else
        {
            hr = E_FAIL;
        }

        pImageWrapper->Release();
    }

    if (FAILED(hr))
        return FALSE;

    return TRUE;
}

HRESULT LoadSkyboxImages()
{
    // create texture2d array for render targets (cube)
    D3D11_TEXTURE2D_DESC cubeMapDesc;
    //cubeMapDesc.Width = 0;
    //cubeMapDesc.Height = 0;
    cubeMapDesc.MipLevels = 1;          // 1, no mips
    cubeMapDesc.ArraySize = 6;          // 6 faces for cubemap
    cubeMapDesc.SampleDesc.Count = 1;
    cubeMapDesc.SampleDesc.Quality = 0;
    cubeMapDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    cubeMapDesc.Usage = D3D11_USAGE_DEFAULT;
    cubeMapDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    cubeMapDesc.CPUAccessFlags = 0;
    cubeMapDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

    //prepare 6 image buffer data
    wchar_t cubeSrc[6][_MAX_PATH] = { L"skybox/right.jpg",
                                      L"skybox/left.jpg",
                                      L"skybox/top.jpg",
                                      L"skybox/bottom.jpg",
                                      L"skybox/front.jpg",
                                      L"skybox/back.jpg" };
    D3D11_SUBRESOURCE_DATA pData[6];
    ImageFrame imgFrame[6];
    for (int i = 0; i < 6; i++) {
        if (!LoadImageFile(cubeSrc[i], imgFrame[i])) {
            OutputDebugStringA("Error: File not found\r\n");
            continue;
        }
        cubeMapDesc.Width = imgFrame[0].unWidth;
        cubeMapDesc.Height = imgFrame[0].unHeight;
        ZeroMemory(&pData[i], sizeof(pData[i]));
        pData[i].pSysMem = imgFrame[i].pBuffer;
        pData[i].SysMemPitch = imgFrame[0].unWidth * 4;
        pData[i].SysMemSlicePitch = 0;
    }

    HRESULT hr = g_pd3dDevice->CreateTexture2D(&cubeMapDesc,
                                               pData,
                                               &g_pCubeTex);
    if (FAILED(hr))
        return hr;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = cubeMapDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.TextureCube.MipLevels = -1;         // -1, use all mips
    srvDesc.TextureCube.MostDetailedMip = 0;    // finest mip = 0

    hr = g_pd3dDevice->CreateShaderResourceView(g_pCubeTex,
                                                &srvDesc,
                                                &g_pCubeTexSR);
    for (int i = 0; i < 6; i++) {
        ReleaseImageBuffer(imgFrame[i]);
    }
    return hr;
}

// https://blog.csdn.net/BonChoix/article/details/8606355
HRESULT InitHDRRenderTarget(UINT cubeSize)
{
    HRESULT hr = S_OK;

    // create texture2d array for render targets (cube)
    D3D11_TEXTURE2D_DESC cubeMapDesc;
    cubeMapDesc.Width = cubeSize;
    cubeMapDesc.Height = cubeSize;
    cubeMapDesc.MipLevels = 0;          // 0, generate all mips
    cubeMapDesc.ArraySize = 6;          // 6 faces for cubemap
    cubeMapDesc.SampleDesc.Count = 1;
    cubeMapDesc.SampleDesc.Quality = 0;
    cubeMapDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    cubeMapDesc.Usage = D3D11_USAGE_DEFAULT;
    cubeMapDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    cubeMapDesc.CPUAccessFlags = 0;
    cubeMapDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS | D3D11_RESOURCE_MISC_TEXTURECUBE;

    hr = g_pd3dDevice->CreateTexture2D(&cubeMapDesc, 
                                       0, 
                                       &g_pCubeTex);
    if (FAILED(hr))
        return hr;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = cubeMapDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.TextureCube.MipLevels = -1;         // -1, use all mips
    srvDesc.TextureCube.MostDetailedMip = 0;    // finest mip = 0

    g_pd3dDevice->CreateShaderResourceView(g_pCubeTex, 
                                           &srvDesc, 
                                           &g_pCubeTexSR);
    if (FAILED(hr))
        return hr;

    // create render target views to draw all cube map faces
    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
    rtvDesc.Format = cubeMapDesc.Format;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
    rtvDesc.Texture2DArray.ArraySize = 1;
    rtvDesc.Texture2DArray.MipSlice = 0;
    for (int i = 0; i < 6; i++) {
        rtvDesc.Texture2DArray.FirstArraySlice = i;
        hr = g_pd3dDevice->CreateRenderTargetView(g_pCubeTex,
                                                  &rtvDesc,
                                                  &g_pCubeMapRTVs[i]);
        if (FAILED(hr))
            return hr;
    }

    // create depth buffer
    D3D11_TEXTURE2D_DESC depthTexDesc;
    depthTexDesc.Width = cubeSize;
    depthTexDesc.Height = cubeSize;
    depthTexDesc.MipLevels = 1;
    depthTexDesc.ArraySize = 1;
    depthTexDesc.SampleDesc.Count = 1;
    depthTexDesc.SampleDesc.Quality = 0;
    depthTexDesc.Format = DXGI_FORMAT_D32_FLOAT;    // depth 32bits, stencil none
    depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
    depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthTexDesc.CPUAccessFlags = 0;
    depthTexDesc.MiscFlags = 0;

    ID3D11Texture2D* depthTex = 0;
    hr = g_pd3dDevice->CreateTexture2D(&depthTexDesc, 
                                       0, 
                                       &depthTex);
    if (FAILED(hr))
        return hr;

    // create the depth stencil view for the entire buffer.
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.Format = depthTexDesc.Format;
    dsvDesc.Flags = 0;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;
    hr = g_pd3dDevice->CreateDepthStencilView(depthTex,
                                              &dsvDesc,
                                              &g_pCubeMapDSV);
    depthTex->Release();
    return hr;
}

HRESULT InitIBLShaders()
{
    HRESULT hr = S_OK;

    // Compile the vertex shader
    hr = CompileShaderFromFile(L"cubmap.fx", "VS_Cubemap", "vs_4_0", &g_pCubeVSBlob);
    if (FAILED(hr)) {
        MessageBox(NULL,
            L"The VS FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    // Create the vertex shader
    hr = g_pd3dDevice->CreateVertexShader(g_pCubeVSBlob->GetBufferPointer(),
                                          g_pCubeVSBlob->GetBufferSize(),
                                          NULL,
                                          &g_pCubeVertexShader);
    if (FAILED(hr)) {
        g_pCubeVSBlob->Release();
        return hr;
    }

    // Compile the pixel shader (PS)
    ID3DBlob* pPSBlob = NULL;
    hr = CompileShaderFromFile(L"cubmap.fx", "PS_Cubmap", "ps_4_0", &pPSBlob);
    if (FAILED(hr))
    {
        MessageBox(NULL,
            L"The PS FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    // Create the pixel shader (PS)
    hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(),
                                         pPSBlob->GetBufferSize(),
                                         NULL,
                                         &g_pCubePixelShader);
    pPSBlob->Release();

    return hr;
}

HRESULT InitSkyboxShaders()
{
    HRESULT hr = S_OK;

    // Compile the vertex shader
    hr = CompileShaderFromFile(L"skybox.fx", "VS_SkyBox", "vs_4_0", &g_pCubeVSBlob);
    if (FAILED(hr))
    {
        MessageBox(NULL,
            L"The VS FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    // Create the vertex shader
    hr = g_pd3dDevice->CreateVertexShader(g_pCubeVSBlob->GetBufferPointer(),
                                          g_pCubeVSBlob->GetBufferSize(),
                                          NULL,
                                          &g_pSkyboxVertexShader);
    if (FAILED(hr)) {
        g_pCubeVSBlob->Release();
        return hr;
    }

    // Compile the pixel shader (PS)
    ID3DBlob* pPSBlob = NULL;
    hr = CompileShaderFromFile(L"skybox.fx", "PS_SkyBox", "ps_4_0", &pPSBlob);
    if (FAILED(hr))
    {
        MessageBox(NULL,
            L"The PS FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    // Create the pixel shader (PS)
    hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(),
                                         pPSBlob->GetBufferSize(),
                                         NULL,
                                         &g_pSkyboxPixelShader);
    pPSBlob->Release();

    return hr;
}


HRESULT InitCubeVertex() {
    HRESULT hr = S_OK;

    // Define the input layout (VAO)
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    UINT numElements = ARRAYSIZE(layout);

    // Set the input layout 
    hr = g_pd3dDevice->CreateInputLayout(layout,
                                         numElements,
                                         g_pCubeVSBlob->GetBufferPointer(),
                                         g_pCubeVSBlob->GetBufferSize(),
                                         &g_pCubeVertexLayout);
    g_pCubeVSBlob->Release();
    if (FAILED(hr))
        return hr;

    // Set vertex buffer (clockwise)
    CubeVertex vertices[24] =
    {
        { XMFLOAT3(-1.f,  1.f, -1.f)},
        { XMFLOAT3(1.f,  1.f, -1.f)},
        { XMFLOAT3(1.f, -1.f, -1.f)},
        { XMFLOAT3(-1.f, -1.f, -1.f)},

        { XMFLOAT3(1.f,  1.f, -1.f)},
        { XMFLOAT3(1.f,  1.f,  1.f)},
        { XMFLOAT3(1.f, -1.f,  1.f)},
        { XMFLOAT3(1.f, -1.f, -1.f)},

        { XMFLOAT3(1.f,  1.f,  1.f)},
        { XMFLOAT3(-1.f,  1.f,  1.f)},
        { XMFLOAT3(-1.f, -1.f,  1.f)},
        { XMFLOAT3(1.f, -1.f,  1.f)},

        { XMFLOAT3(-1.f,  1.f, 1.f)},
        { XMFLOAT3(-1.f,  1.f, -1.f)},
        { XMFLOAT3(-1.f, -1.f, -1.f)},
        { XMFLOAT3(-1.f, -1.f, 1.f)},

        { XMFLOAT3(-1.f,  1.f, 1.f)},
        { XMFLOAT3(1.f,  1.f, 1.f)},
        { XMFLOAT3(1.f,  1.f, -1.f)},
        { XMFLOAT3(-1.f, 1.f, -1.f)},

        { XMFLOAT3(1.f, -1.f, -1.f)},
        { XMFLOAT3(1.f, -1.f,  1.f)},
        { XMFLOAT3(-1.f, -1.f, 1.f)},
        { XMFLOAT3(-1.f, -1.f, -1.f)},
    };

    UINT numVertexElements = ARRAYSIZE(vertices);

    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(CubeVertex) * numVertexElements;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = vertices;
    hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pCubeVertexBuffer);
    if (FAILED(hr))
        return hr;

    // Set index buffer
    WORD indices[] =
    { 
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        8, 9, 10, 10, 11, 8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20 
    };
    UINT numIndices = ARRAYSIZE(indices);

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(WORD) * numIndices;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;

    bd.CPUAccessFlags = 0;
    InitData.pSysMem = indices;
    hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &g_pCubeIndexBuffer);

    return hr;
}


HRESULT LoadHDRTexture()
{
    HRESULT hr = S_OK;

    // Load the Texture
    DirectX::TexMetadata md;
    DirectX::ScratchImage img;
    WCHAR filepath[] = L"newport_loft.hdr";
    hr = LoadFromHDRFile(filepath,
                         &md,
                         img);
    if (FAILED(hr))
        return hr;

    hr = CreateShaderResourceView(g_pd3dDevice,
                                  img.GetImages(),
                                  img.GetImageCount(),
                                  md,
                                  &g_pHDRTextureRV);
    if (FAILED(hr))
        return hr;

    hr = CreateSamplerLinear();
    return hr;
}

HRESULT InitIBLConstBuffer()
{
    HRESULT hr = S_OK;

    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(ConstCubeBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = g_pd3dDevice->CreateBuffer(&bd, NULL, &g_pCubeConstBuffer);
    if (FAILED(hr))
        return hr;

    // Initialize the camera view matrix, 6 directions
    XMVECTOR Eye = XMVectorSet(0.f, 0.f, 0.f, 0.0f);
    XMVECTOR At[6] =
    {
        XMVectorSet( 1.f,  0.f,  0.f, 0.f),     // +X
        XMVectorSet(-1.f,  0.f,  0.f, 0.f),     // -X
        XMVectorSet( 0.f,  1.f,  0.f, 0.f),     // +Y
        XMVectorSet( 0.f, -1.f,  0.f, 0.f),     // -Y
        XMVectorSet( 0.f,  0.f,  1.f, 0.f),     // +Z
        XMVectorSet( 0.f,  0.f, -1.f, 0.f)      // -Z
    };
    XMVECTOR Up[6] =
    {
        XMVectorSet(0.f, 1.f,  0.f, 0.f),       // +X
        XMVectorSet(0.f, 1.f,  0.f, 0.f),       // -X
        XMVectorSet(0.f, 0.f, -1.f, 0.f),       // +Y
        XMVectorSet(0.f, 0.f,  1.f, 0.f),       // -Y
        XMVectorSet(0.f, 1.f,  0.f, 0.f),       // +Z
        XMVectorSet(0.f, 1.f,  0.f, 0.f)        // -Z
    };
    for (int i = 0; i < 6; i++) {
        g_CubeViews[i] = XMMatrixLookAtLH(Eye, At[i], Up[i]);
    }

    // Initialize the projection matrix
    float aspect_ratio = 1.f;
    g_CubeProjection = XMMatrixPerspectiveFovLH(XM_PIDIV2, aspect_ratio, 0.01f, 100.f);

    return hr;
}

void CreateSkyboxDepthStencilState()
{
    D3D11_DEPTH_STENCIL_DESC dssDesc;
    ZeroMemory(&dssDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
    dssDesc.DepthEnable = true;
    dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dssDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;    // must set as LESS_EQUAL

    g_pd3dDevice->CreateDepthStencilState(&dssDesc, &g_pSkyboxDepthState);
    return;
}

HRESULT CreateSkyboxRasterState()
{
    CreateSkyboxDepthStencilState();

    D3D11_RASTERIZER_DESC RSState;
    RSState.FillMode = D3D11_FILL_SOLID;
    RSState.CullMode = D3D11_CULL_BACK;
    RSState.FrontCounterClockwise = TRUE;
    RSState.DepthBias = 0;
    RSState.DepthBiasClamp = 0.0f;
    RSState.SlopeScaledDepthBias = 0.0f;
    RSState.DepthClipEnable = FALSE;
    RSState.ScissorEnable = FALSE;
    RSState.MultisampleEnable = TRUE;
    RSState.AntialiasedLineEnable = TRUE;

    HRESULT hr = g_pd3dDevice->CreateRasterizerState(&RSState, &g_pSkyboxRasterizerState);
    return hr;
}

void RenderCube(UINT);
void DrawCubeMap(UINT cubeMapSize)
{
    SetViewPort(cubeMapSize, cubeMapSize);
    g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


    UINT stride = sizeof(CubeVertex);
    UINT offset = 0;    
    g_pImmediateContext->IASetInputLayout(g_pCubeVertexLayout);
    g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pCubeVertexBuffer, &stride, &offset);
    g_pImmediateContext->IASetIndexBuffer(g_pCubeIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    // Generate the cube map.
    g_pImmediateContext->VSSetShader(g_pCubeVertexShader, NULL, 0);
    g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCubeConstBuffer);

    g_pImmediateContext->PSSetShader(g_pCubePixelShader, NULL, 0);

    g_pImmediateContext->PSSetShaderResources(0, 1, &g_pHDRTextureRV);
    g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);

    for (int i = 0; i < 6; ++i)  {
        RenderCube(i);
    }
    g_pImmediateContext->GenerateMips(g_pCubeTexSR);
}

void RenderCube(UINT face)
{
    // Clear cube map face and depth buffer.
    // Clear the back buffer &  depth buffer
    float ClearColor[4] = { 0.f, 1.f, 0.f, 1.f }; // red, green, blue, alpha
    g_pImmediateContext->ClearRenderTargetView(g_pCubeMapRTVs[face], ClearColor);
    g_pImmediateContext->ClearDepthStencilView(g_pCubeMapDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

    // Bind cube map face as render target.
    //ID3D11RenderTargetView** renderTargets;
    //renderTargets = &g_pCubeMapRTVs[face];
    g_pImmediateContext->OMSetRenderTargets(1, &g_pCubeMapRTVs[face], g_pCubeMapDSV);

    XMMATRIX world = XMMatrixIdentity();

    ConstCubeBuffer cb;
    cb.mWorld = XMMatrixTranspose(world);
    cb.mView = XMMatrixTranspose(g_CubeViews[face]);
    cb.mProjection = XMMatrixTranspose(g_CubeProjection);
    g_pImmediateContext->UpdateSubresource(g_pCubeConstBuffer, 0, NULL, &cb, 0, 0);

    g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantBuffer);
    g_pImmediateContext->DrawIndexed(36, 0, 0);
}

void RenderSkybox()
{
    UINT stride = sizeof(CubeVertex);
    UINT offset = 0;
    g_pImmediateContext->IASetInputLayout(g_pCubeVertexLayout);
    g_pImmediateContext->IASetVertexBuffers(0, 1, &g_pCubeVertexBuffer, &stride, &offset);
    g_pImmediateContext->IASetIndexBuffer(g_pCubeIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    XMMATRIX world = XMMatrixIdentity();

    ConstCubeBuffer cb;
    cb.mWorld = XMMatrixTranspose(world);
    cb.mView = XMMatrixTranspose(g_View);
    cb.mProjection = XMMatrixTranspose(g_Projection);

    g_pImmediateContext->UpdateSubresource(g_pCubeConstBuffer, 0, NULL, &cb, 0, 0);
    
    g_pImmediateContext->VSSetShader(g_pSkyboxVertexShader, NULL, 0);
    g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pCubeConstBuffer);

    g_pImmediateContext->RSSetState(g_pSkyboxRasterizerState);
    g_pImmediateContext->OMSetDepthStencilState(g_pSkyboxDepthState, 0);  //must-have

    g_pImmediateContext->PSSetShader(g_pSkyboxPixelShader, NULL, 0);

    g_pImmediateContext->PSSetShaderResources(0, 1, &g_pCubeTexSR);
    g_pImmediateContext->PSSetSamplers(0, 1, &g_pSamplerLinear);
    g_pImmediateContext->DrawIndexed(36, 0, 0);

    g_pImmediateContext->OMSetDepthStencilState(0, 0);
}

void CleanupIBL()
{
    if (g_pHDRTextureRV) g_pHDRTextureRV->Release();
    if (g_pCubeConstBuffer) g_pCubeConstBuffer->Release();
    if (g_pCubeVertexBuffer) g_pCubeVertexBuffer->Release();
    if (g_pCubeIndexBuffer) g_pCubeIndexBuffer->Release();
    if (g_pCubeVertexLayout) g_pCubeVertexLayout->Release();
    if (g_pCubeVertexShader) g_pCubeVertexShader->Release();
    if (g_pCubePixelShader) g_pCubePixelShader->Release();
    if (g_pCubeMapDSV) g_pCubeMapDSV->Release();
    for (int i = 0; i < 6; i++) {
        if (g_pCubeMapRTVs[i]) g_pCubeMapRTVs[i]->Release();
    }
    if (g_pSkyboxVertexShader) g_pSkyboxVertexShader->Release();
    if (g_pSkyboxPixelShader) g_pSkyboxPixelShader->Release();
    if (g_pCubeTex) g_pCubeTex->Release();
    if (g_pCubeTexSR) g_pCubeTexSR->Release();

}