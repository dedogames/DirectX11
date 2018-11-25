#include<windows.h>
//https://www.braynzarsoft.net/viewtutorial/q16390-directx-11-an-introduction-to-the-win32-api
//Include and link appropriate libraries and headers//



#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>
#include <xnamath.h>

/* Variaveis Globais */
/* Armazena os dados que serão renderizados na tela
   O que ocorre é uma troca(Swap) de buffers(font e back) para
   que não ocorra sobreposição de imagens.
 
*/
IDXGISwapChain* SwapChain; 

/* Representação da placa de video(GPU), com ele é possivel saber
  as capacidades da placa

  O DirectX 11 implemetou um conceito multithread para 
  melhroar na performance.
  Pense em um modelo basico de MVC:

  Model: d3d11Device
  Controller: d3d11DevCon
  View: renderTargetView(backbuffer)
*/
ID3D11Device* d3d11Device; 
/* Execulta os methodos de renderização */
ID3D11DeviceContext * d3d11DevCon;

LPCTSTR WndClassName = "WndClassName";
float red = 0.0f;
float green = 0.0f;
float blue = 0.0f;
int colormodr = 1;
int colormodg = 1;
int colormodb = 1;

int Width = 800;
int Height = 600;

HWND hwnd;

bool InitializeDirect3d11App(HINSTANCE HInstance);
void ReleaseObjects();
bool InitScene();
void UpdateScene();
void DrawScene();
int messageloop();
LRESULT CALLBACK WndProc(HWND hwnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam);


ID3D11RenderTargetView* renderTargetView;
 

bool InitializeWindow(HINSTANCE hInstance,
	int ShowWnd,
	int width, int height,
	bool windowed)
{
	typedef struct _WNDCLASS {
		UINT cbSize;
		UINT style;
		WNDPROC lpfnWndProc;
		int cbClsExtra;
		int cbWndExtra;
		HANDLE hInstance;
		HICON hIcon;
		HCURSOR hCursor;
		HBRUSH hbrBackground;
		LPCTSTR lpszMenuName;
		LPCTSTR lpszClassName;
	} WNDCLASS;

	WNDCLASSEX wc;

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = NULL;
	wc.cbWndExtra = NULL;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = WndClassName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, "Error registering class",
			"Error", MB_OK | MB_ICONERROR);
		return 1;
	}

	hwnd = CreateWindowEx(
		NULL,
		WndClassName,
		"Window Title",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		width, height,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (!hwnd)
	{
		MessageBox(NULL, "Error creating window",
			"Error", MB_OK | MB_ICONERROR);
		return 1;
	}

	ShowWindow(hwnd, ShowWnd);
	UpdateWindow(hwnd);

	return true;
}

int WINAPI WinMain( 
	HINSTANCE hInstance,  /**/
	HINSTANCE hPrevInstance,  /**/
	LPSTR lpCmdLine,  /**/
	int nShowCmd /**/)
{

	if (!InitializeWindow(hInstance, nShowCmd, Width, Height, true))
	{
		MessageBox(0, "Direct 3D Initialization- Failed", "Error", MB_OK);
		return 0;
	}

	if (!InitializeDirect3d11App(hInstance))
	{
		MessageBox(0, "Direct 3D Initialization- Failed", "Error", MB_OK);
		return 0;
	}

	if (!InitScene())
	{
		MessageBox(0, "Scene Initialization ","Error", MB_OK);
		return 0;
	}


	messageloop();

	ReleaseObjects();
	return 0;

}



bool InitializeDirect3d11App(HINSTANCE HInstance)
{
	HRESULT hr;

	/* Descrevendo o buffer */
	DXGI_MODE_DESC bufferDesc;
	/*  DXGI_MODE_DESC
		https://msdn.microsoft.com/en-us/library/windows/desktop/bb173064(v=vs.85).aspx
	*/


	ZeroMemory(&bufferDesc, sizeof(DXGI_MODE_DESC));
	bufferDesc.Width       = Width;
	bufferDesc.Height      = Height;
	bufferDesc.RefreshRate.Denominator = 1;
	bufferDesc.RefreshRate.Numerator = 60;	
	bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;



	//Descrive SwapChain
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

	swapChainDesc.BufferDesc = bufferDesc;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 5;
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	hr = D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL, 
		NULL, 
		NULL,
		NULL, 
		D3D11_SDK_VERSION, 
		&swapChainDesc,
		&SwapChain,
		&d3d11Device,
		NULL,
		&d3d11DevCon);

	ID3D11Texture2D* BackBuffer;
	hr = SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&BackBuffer);


	hr = d3d11Device->CreateRenderTargetView(BackBuffer, NULL, &renderTargetView);
	BackBuffer->Release();

	d3d11DevCon->OMSetRenderTargets(1, &renderTargetView, NULL);
	
	return true; 
}

void ReleaseObjects()
{
 
    //Release the COM Objects we created
    SwapChain->Release();
    d3d11Device->Release();
    d3d11DevCon->Release(); 
}

bool InitScene()
{
	return true;
}

void UpdateScene()
{
	
		//Update the colors of our scene
		red += colormodr * 0.00005f;
		green += colormodg * 0.00002f;
		blue += colormodb * 0.00001f;

		if (red >= 1.0f || red <= 0.0f)
			colormodr *= -1;
		if (green >= 1.0f || green <= 0.0f)
			colormodg *= -1;
		if (blue >= 1.0f || blue <= 0.0f)
			colormodb *= -1;
	
}

void DrawScene()
{
	 
		//Clear our backbuffer to the updated color
		D3DXCOLOR bgColor(red, green, blue, 1.0f);

		d3d11DevCon->ClearRenderTargetView(renderTargetView, bgColor);

		//Present the backbuffer to the screen
		SwapChain->Present(0, 0); 
}


int messageloop() {
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));
	while (true)
	{
		BOOL PeekMessageL(
			LPMSG lpMsg,
			HWND hWnd,
			UINT wMsgFilterMin,
			UINT wMsgFilterMax,
			UINT wRemoveMsg
		);

		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			///////////////**************new**************////////////////////
					// run game code

			UpdateScene();
			DrawScene();

			///////////////**************new**************////////////////////
		}
	}
	return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam)
{
	switch (msg)
	{
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) {
			DestroyWindow(hwnd);
		}
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd,
		msg,
		wParam,
		lParam);
}

