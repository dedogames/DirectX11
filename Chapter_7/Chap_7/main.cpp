#include<windows.h>
//https://www.braynzarsoft.net/viewtutorial/q16390-directx-11-an-introduction-to-the-win32-api
//Include and link appropriate libraries and headers//



#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>
#include <xnamath.h>
 
IDXGISwapChain* SwapChain;  
ID3D11Device* d3d11Device;  
ID3D11DeviceContext * d3d11DevCon;
ID3D11RenderTargetView* renderTargetView;

ID3D11DepthStencilView * depthStencilView;
ID3D11Texture2D* depthStencilBuffer;


ID3D11Buffer* triangleVertexBuffer;
ID3D11VertexShader* VS;
ID3D11PixelShader* PS;
ID3D10Blob* VS_Buffer;
ID3D10Blob* PS_Buffer;
ID3D11InputLayout* vertLayout;
D3D11_VIEWPORT vps[2];

ID3D11Buffer* squareIndexBuffer;
ID3D11Buffer* squareVertBuffer;
ID3D11Buffer* cbPerObjectBuffer;

XMMATRIX WVP;
XMMATRIX cube1World;
XMMATRIX cube2World;
XMMATRIX World;
XMMATRIX CamView;
XMMATRIX camProjection;

XMVECTOR camPosition;
XMVECTOR camTarget;
XMVECTOR camUP;

XMMATRIX Rotation;
XMMATRIX Scale;
XMMATRIX Translation;
float rot = 0.01f;


LPCTSTR WndClassName = "WndClassName";
 

int Width = 300;
int Height = 300;

HWND hwnd;
HRESULT hr;
/* Vertex structure and Vertex Layout(input layout) */
struct Vertex
{
	Vertex() {}
	Vertex(float x, float y, float z, 
		   float cr, float cg, float cb, float ca
		) : pos(x, y, z),color(cr,cg,cb,ca){}
	XMFLOAT3 pos;
	XMFLOAT4 color;
};

struct cbPerObject
{
	XMMATRIX  WVP;
};

cbPerObject cbPerObj;

D3D11_INPUT_ELEMENT_DESC layout[] =
{
	{"POSITION", 0 , DXGI_FORMAT_R32G32B32_FLOAT, 0,0,D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{"COLOR", 0 , DXGI_FORMAT_R32G32B32_FLOAT, 0,12,D3D11_INPUT_PER_VERTEX_DATA, 0 }
};
UINT numelements = ARRAYSIZE(layout);

bool InitializeDirect3d11App(HINSTANCE HInstance);
//void ReleaseObjects();
bool InitScene();
void UpdateScene();
void DrawScene();
int messageloop();
void CleanUp();
LRESULT CALLBACK WndProc(HWND hwnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam);


 

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
	CleanUp();
	 
	return 0;

}


void CleanUp()
{
	SwapChain->Release();
	d3d11DevCon->Release();
	d3d11Device->Release();
	renderTargetView->Release();
	triangleVertexBuffer->Release();
	VS->Release();
	PS->Release();
	VS_Buffer->Release();
	PS_Buffer->Release();
	vertLayout->Release();
	depthStencilView->Release();
	depthStencilBuffer->Release();
	cbPerObjectBuffer->Release();
}


bool InitializeDirect3d11App(HINSTANCE HInstance)
{
	 

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
	swapChainDesc.BufferCount = 1;
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

	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = Width;
	depthStencilDesc.Height = Height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	d3d11Device->CreateTexture2D(&depthStencilDesc, NULL, &depthStencilBuffer);
	d3d11Device->CreateDepthStencilView(depthStencilBuffer, NULL, &depthStencilView);

	d3d11DevCon->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
	
	return true; 
}

 

bool InitScene()
{
	hr = D3DX11CompileFromFile("Effects.fx", 0, 0, "VS", "vs_4_0", 0, 0, 0, &VS_Buffer, 0, 0);
	hr = D3DX11CompileFromFile("Effects.fx", 0, 0, "PS", "ps_4_0", 0, 0, 0, &PS_Buffer, 0, 0);

	//Create the shader Objects
	hr = d3d11Device->CreateVertexShader(VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), NULL, &VS);
	hr = d3d11Device->CreatePixelShader(PS_Buffer->GetBufferPointer(), PS_Buffer->GetBufferSize(), NULL, &PS);	 

	//Set Vertex and PIexel shaders
	d3d11DevCon->VSSetShader(VS, 0, 0);
	d3d11DevCon->PSSetShader(PS, 0, 0);

	//Create the vertex buffer
	Vertex v[] =
	{
  Vertex(-1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f),
		Vertex(-1.0f, +1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f),
		Vertex(+1.0f, +1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f),
		Vertex(+1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f),
		Vertex(-1.0f, -1.0f, +1.0f, 0.0f, 1.0f, 1.0f, 1.0f),
		Vertex(-1.0f, +1.0f, +1.0f, 1.0f, 1.0f, 1.0f, 1.0f),
		Vertex(+1.0f, +1.0f, +1.0f, 1.0f, 0.0f, 1.0f, 1.0f),
		Vertex(+1.0f, -1.0f, +1.0f, 1.0f, 0.0f, 0.0f, 1.0f),
		 
	};

	DWORD indices[] = {
		// front face
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3,
		4, 3, 7
	};

	D3D11_BUFFER_DESC indexBufferDes;
	ZeroMemory(&indexBufferDes, sizeof(indexBufferDes));

	indexBufferDes.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDes.ByteWidth = sizeof(DWORD) * 12 * 3;
	indexBufferDes.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDes.CPUAccessFlags = 0;
	indexBufferDes.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData; 
	iinitData.pSysMem = indices;
	d3d11Device->CreateBuffer(&indexBufferDes, &iinitData, &squareIndexBuffer);
	d3d11DevCon->IASetIndexBuffer(squareIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
	vertexBufferDesc.Usage			= D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth		= sizeof(Vertex) *8;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags		= 0;

	D3D11_SUBRESOURCE_DATA vertexBufferData;
	ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
	vertexBufferData.pSysMem = v;
	hr = d3d11Device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &triangleVertexBuffer);

	//set the vertex buffer
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	d3d11DevCon->IASetVertexBuffers(0, 1, &triangleVertexBuffer, &stride, &offset);

	//Create the Input Layout
	d3d11Device->CreateInputLayout(layout,
		numelements,
		VS_Buffer->GetBufferPointer(),
		VS_Buffer->GetBufferSize(),&vertLayout);

	d3d11DevCon->IASetInputLayout(vertLayout);

	 
	//Set primitive topology
	d3d11DevCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//Create the view port
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
	viewport.TopLeftX = 0;
	viewport.TopLeftX = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.Width = 300;
	viewport.Height = 300;


	//Create the view port
	D3D11_VIEWPORT viewport2;
	ZeroMemory(&viewport2, sizeof(D3D11_VIEWPORT));	
	viewport2.TopLeftX = 100;
	viewport2.TopLeftX = 100;
	viewport2.Width = 300;
	viewport2.Height = 300;

	vps[0] = viewport;
	vps[1] = viewport2;
 
	//Set the viewport	
	d3d11DevCon->RSSetViewports(2, vps);

	//Create the buffer to send to the cBuffer in effect file
	D3D11_BUFFER_DESC cbbd;
	ZeroMemory(&cbbd, sizeof(D3D11_BUFFER_DESC));
	cbbd.Usage = D3D11_USAGE_DEFAULT;
	cbbd.ByteWidth = sizeof(cbPerObject);
	cbbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbbd.CPUAccessFlags = 0;
	cbbd.MiscFlags = 0;

	hr = d3d11Device->CreateBuffer(&cbbd, NULL, &cbPerObjectBuffer);

	//Camera Information
	camPosition = XMVectorSet(0.0f,3.0f, -8.0f, 0.0f);
	camTarget   = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	camUP       = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	//set the vie wmatrix
	CamView = XMMatrixLookAtLH(camPosition, camTarget, camUP);
	//Set the projectil matrix
	camProjection = XMMatrixPerspectiveFovLH(0.4f * 3.14f, (float)Width / Height, 1.0f, 1000.0f);
	

	return true;


}

void UpdateScene()
{
	rot += .0005f;
	if (rot > 6.26f)
		rot = 0.0f;

	//Reset Cube1world
	cube1World = XMMatrixIdentity();
	//Define cub1 world space matrix
	XMVECTOR rotaxis = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	Rotation = XMMatrixRotationAxis(rotaxis, rot);
	Translation = XMMatrixTranslation(0.0f, 0.0f, 5.0f);

	//Set cube1 world space using the transformations
	cube1World = Translation * Rotation;

	//Reset cube2World
	cube2World = XMMatrixIdentity();

	//Define cube world space matrix
	Rotation = XMMatrixRotationAxis(rotaxis, -rot);
	Scale = XMMatrixScaling(1.3f, 1.3f, 1.3f);
	//Set cube world space matrix
	cube2World = Rotation * Scale;
	
 
	
}

void DrawScene()
{
	 
		//Clear our backbuffer to the updated color
	float bgColor[4] = { (0.0f, 0.0f, 0.0f, 0.0f) };
	d3d11DevCon->ClearRenderTargetView(renderTargetView, bgColor);
	d3d11DevCon->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);


	// Set WVP matrix and send it to the constant buffer in effect file//
	
	WVP = cube1World * CamView * camProjection;
	cbPerObj.WVP = XMMatrixTranspose(WVP);
	d3d11DevCon->UpdateSubresource(cbPerObjectBuffer, 0, NULL, &cbPerObj, 0, 0);
	d3d11DevCon->VSSetConstantBuffers(0, 1, &cbPerObjectBuffer);

	//Draw first cube
	d3d11DevCon->DrawIndexed(36, 0, 0);

	WVP = cube2World * CamView * camProjection;
	cbPerObj.WVP = XMMatrixTranspose(WVP);
	d3d11DevCon->UpdateSubresource(cbPerObjectBuffer, 0, NULL, &cbPerObj, 0, 0);
	d3d11DevCon->VSSetConstantBuffers(0, 1, &cbPerObjectBuffer);

	//Draw Cube 2
	d3d11DevCon->DrawIndexed(36, 0, 0);



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

