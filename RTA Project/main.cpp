#include "stdafx.h"
#include "Importer.h"

class RTA_PROJECT
{
	HINSTANCE						application;
	WNDPROC							appWndProc;
	HWND							window;

	CComPtr<ID3D11Device> device;
	CComPtr<ID3D11DeviceContext> deviceContext;
	CComPtr<ID3D11RenderTargetView> rtv;
	CComPtr<IDXGISwapChain> swapchain;
	D3D11_VIEWPORT viewport;

	CComPtr<ID3D11Buffer> vertexBuffer, vertexBufferPlane, indexBuffer, indexBufferPlane, constantBufferObj, constantBufferScene;

	/*CComPtr<ID3D11Buffer> vertexBufferPlane;
	CComPtr<ID3D11Buffer> indexBuffer;
	CComPtr<ID3D11Buffer> indexBufferPlane;
	CComPtr<ID3D11Buffer> constantBufferObj;
	CComPtr<ID3D11Buffer> constantBufferScene;*/

	CComPtr<ID3D11InputLayout> input;

	CComPtr<ID3D11VertexShader> vShader;

	CComPtr<ID3D11PixelShader> pShader;

	CComPtr<ID3D11DepthStencilView> dsView;

	CComPtr<ID3D11Texture2D> depthStencil;

	CComPtr<ID3D11ShaderResourceView> srv;

	CComPtr<ID3D11SamplerState> sampler;

	CComPtr<ID3D11RasterizerState> rasterstate;

	CComPtr<ID3D11Resource> texture;

	OBJECT model, plane;

	SCENE camera;

	Importer import;

	vector<OBJVERTEX> v_model, v_plane;
	vector<UINT> v_modelCount, v_planeCount;

	float aspectRatio = (float)(BACKBUFFER_WIDTH) / (BACKBUFFER_HEIGHT);
	float zNear = 0.1f;
	float zFar = 100.0f;
	float zBuffer[PIXELS];
	UINT Raster[PIXELS];

	float Degrees_to_Radian(float Deg)
	{
		return Deg * PI / 180.0f;
	}

	float clearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };

public:

	RTA_PROJECT(HINSTANCE hinst, WNDPROC proc);
	bool Run();
	bool ShutDown();
	
};

RTA_PROJECT::RTA_PROJECT(HINSTANCE hinst, WNDPROC proc)
{
	import.ImportFile("Teddy_Idle.fbx");

	application = hinst;
	appWndProc = proc;

	WNDCLASSEX  wndClass;
	ZeroMemory(&wndClass, sizeof(wndClass));
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.lpfnWndProc = appWndProc;
	wndClass.lpszClassName = L"DirectXApplication";
	wndClass.hInstance = application;
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOWFRAME);
	//wndClass.hIcon			= LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_FSICON));
	RegisterClassEx(&wndClass);

	RECT window_size = { 0, 0, BACKBUFFER_WIDTH, BACKBUFFER_HEIGHT };
	AdjustWindowRect(&window_size, WS_OVERLAPPEDWINDOW, false);

	window = CreateWindow(L"DirectXApplication", L"RTA Project", WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX),
		CW_USEDEFAULT, CW_USEDEFAULT, window_size.right - window_size.left, window_size.bottom - window_size.top,
		NULL, NULL, application, this);

	ShowWindow(window, SW_SHOW);

	DXGI_SWAP_CHAIN_DESC swap_chain;
	ZeroMemory(&swap_chain, sizeof(DXGI_SWAP_CHAIN_DESC));
	swap_chain.BufferCount = 1;
	swap_chain.BufferDesc.Width = BACKBUFFER_WIDTH;
	swap_chain.BufferDesc.Height = BACKBUFFER_HEIGHT;
	swap_chain.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swap_chain.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swap_chain.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swap_chain.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain.SampleDesc.Count = 4;
	swap_chain.OutputWindow = window;
	swap_chain.Windowed = true;
	swap_chain.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swap_chain.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; //alt+enter fullscreen

	D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG, NULL, 0, D3D11_SDK_VERSION, &swap_chain, &swapchain, &device, NULL, &deviceContext);

	ID3D11Resource *resource;

	swapchain->GetBuffer(0, __uuidof(resource),
		reinterpret_cast<void**>(&resource));

	device->CreateRenderTargetView(resource, NULL, &rtv);

	resource->Release();

	// Viewport description
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = BACKBUFFER_WIDTH;
	viewport.Height = BACKBUFFER_HEIGHT;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;

	// Vertex buffer
	D3D11_BUFFER_DESC model_vbuffer;
	ZeroMemory(&model_vbuffer, sizeof(D3D11_BUFFER_DESC));
	model_vbuffer.ByteWidth = sizeof(Importer::Vertex) * import.totalVertexes.size();
	model_vbuffer.Usage = D3D11_USAGE_IMMUTABLE;
	model_vbuffer.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	model_vbuffer.StructureByteStride = sizeof(Importer::Vertex);

	D3D11_SUBRESOURCE_DATA data_1;
	ZeroMemory(&data_1, sizeof(D3D11_SUBRESOURCE_DATA));
	data_1.pSysMem = import.totalVertexes.data();

	device->CreateBuffer(&model_vbuffer, &data_1, &vertexBuffer.p);

	// Index buffer
	D3D11_BUFFER_DESC model_ibuffer;
	ZeroMemory(&model_ibuffer, sizeof(D3D11_BUFFER_DESC));
	model_ibuffer.Usage = D3D11_USAGE_IMMUTABLE;
	model_ibuffer.ByteWidth = sizeof(UINT) * import.polygonCount;
	model_ibuffer.BindFlags = D3D11_BIND_INDEX_BUFFER;
	model_ibuffer.StructureByteStride = sizeof(UINT);

	D3D11_SUBRESOURCE_DATA idata;
	ZeroMemory(&idata, sizeof(D3D11_SUBRESOURCE_DATA));
	idata.pSysMem = import.controlPoints.data();

	device->CreateBuffer(&model_ibuffer, &idata, &indexBuffer);

	D3D11_TEXTURE2D_DESC model_depth;
	ZeroMemory(&model_depth, sizeof(D3D11_TEXTURE2D_DESC));

	model_depth.Width = BACKBUFFER_WIDTH;
	model_depth.Height = BACKBUFFER_HEIGHT;
	model_depth.MipLevels = 1;
	model_depth.ArraySize = 1;
	model_depth.Format = DXGI_FORMAT_D32_FLOAT;
	model_depth.SampleDesc.Count = 4;
	model_depth.Usage = D3D11_USAGE_DEFAULT;
	model_depth.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	device->CreateTexture2D(&model_depth, NULL, &depthStencil.p);

	D3D11_DEPTH_STENCIL_VIEW_DESC model_dsvd;
	ZeroMemory(&model_dsvd, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	model_dsvd.Format = DXGI_FORMAT_D32_FLOAT;
	model_dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	model_dsvd.Texture2D.MipSlice = 0;

	device->CreateDepthStencilView(depthStencil, &model_dsvd, &dsView.p);

	device->CreateVertexShader(RTA_VS, sizeof(RTA_VS), nullptr, &vShader.p);
	device->CreatePixelShader(RTA_PS, sizeof(RTA_PS), nullptr, &pShader.p);

	D3D11_INPUT_ELEMENT_DESC model_Layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "UV", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMALS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	device->CreateInputLayout(model_Layout, ARRAYSIZE(model_Layout), RTA_VS, sizeof(RTA_VS), &input.p);

	D3D11_BUFFER_DESC cbufferObject;
	ZeroMemory(&cbufferObject, sizeof(D3D11_BUFFER_DESC));

	cbufferObject.ByteWidth = sizeof(OBJECT);
	cbufferObject.Usage = D3D11_USAGE_DYNAMIC;
	cbufferObject.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbufferObject.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbufferObject.StructureByteStride = sizeof(float);

	device->CreateBuffer(&cbufferObject, NULL, &constantBufferObj.p);

	D3D11_BUFFER_DESC cbufferScene;
	ZeroMemory(&cbufferScene, sizeof(D3D11_BUFFER_DESC));

	cbufferScene.ByteWidth = sizeof(SCENE);
	cbufferScene.Usage = D3D11_USAGE_DYNAMIC;
	cbufferScene.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbufferScene.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbufferScene.StructureByteStride = sizeof(float);

	device->CreateBuffer(&cbufferScene, NULL, &constantBufferScene.p);

	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&sampler, sizeof(D3D11_SAMPLER_DESC));

	samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_GREATER;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&samplerDesc, &sampler.p);
}

bool RTA_PROJECT::Run()
{
	deviceContext->ClearRenderTargetView(rtv, clearColor);
	deviceContext->OMSetRenderTargets(1, &rtv.p, dsView);
	deviceContext->ClearDepthStencilView(dsView, D3D11_CLEAR_DEPTH, 1, 0);

	deviceContext->VSSetConstantBuffers(0, 1, &constantBufferObj.p);
	deviceContext->VSSetConstantBuffers(1, 1, &constantBufferScene.p);

	D3D11_MAPPED_SUBRESOURCE map;
	deviceContext->Map(constantBufferObj, 0, D3D11_MAP_WRITE_DISCARD, 0, &map);
	memcpy(map.pData, &camera, sizeof(SCENE));
	deviceContext->Unmap(constantBufferScene, 0);

	deviceContext->RSSetViewports(1, &viewport);

	UINT stridesize = sizeof(OBJVERTEX);
	UINT strideoffset = 0;
	deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer.p, &stridesize, &strideoffset);
	deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	deviceContext->PSSetShaderResources(0, 1, &srv.p);

	deviceContext->VSSetShader(vShader, nullptr, 0);
	deviceContext->PSSetShader(pShader, nullptr, 0);

	deviceContext->IASetInputLayout(input);
	
	deviceContext->DrawIndexed(import.totalVertexes.size(), 0, 0);

	swapchain->Present(0, 0);
	return true;
}

bool RTA_PROJECT::ShutDown()
{
	UnregisterClass(L"DirectXApplication", application);
	return true;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wparam, LPARAM lparam);
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR, int)
{
	srand(unsigned int(time(0)));
	RTA_PROJECT myApp(hInstance, (WNDPROC)WndProc);
	MSG msg; ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT && myApp.Run())
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	myApp.ShutDown();
	return 0;
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (GetAsyncKeyState(VK_ESCAPE))
		message = WM_DESTROY;
	switch (message)
	{
	case (WM_DESTROY): { PostQuitMessage(0); }
					   break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}
