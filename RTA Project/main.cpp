#include "stdafx.h"
#include "Importer.h"

float aspectRatio = (float)(BACKBUFFER_WIDTH) / (BACKBUFFER_HEIGHT);
float zNear = 0.1f;
float zFar = 100.0f;
float zBuffer[PIXELS];
UINT Raster[PIXELS];

float clearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };

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

	CComPtr<ID3D11Buffer> vertexBuffer, vertexBufferPlane, vertexBufferSkeleton, indexBuffer, indexBufferPlane, indexBufferSkeleton, constantBufferObj, constantBufferScene, constantBufferDirectional, constantBufferSkeleton;

	CComPtr<ID3D11InputLayout> input;

	CComPtr<ID3D11VertexShader> vShader;

	CComPtr<ID3D11PixelShader> pShader;

	CComPtr<ID3D11DepthStencilView> dsView;

	CComPtr<ID3D11Texture2D> depthStencil;

	CComPtr<ID3D11ShaderResourceView> srv;

	CComPtr<ID3D11SamplerState> sampler;

	CComPtr<ID3D11RasterizerState> rasterstate;

	CComPtr<ID3D11Resource> texture;

	OBJECT model, plane, bones;

	SCENE camera;

	Importer import, skele;

	DIRECTIONAL_LIGHT direction;

	POINT currPos;

	XTime timer;

	

public:

	RTA_PROJECT(HINSTANCE hinst, WNDPROC proc);
	bool Run();
	bool ShutDown();
	void Camera_Movement();
	void Sun();

	/*const VERTEX Skeleton[]
	{
		{}
	}*/
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

#pragma region Swap Chain
	DXGI_SWAP_CHAIN_DESC swap_chain;
	ZeroMemory(&swap_chain, sizeof(DXGI_SWAP_CHAIN_DESC));
	swap_chain.BufferCount = 1;
	swap_chain.BufferDesc.Width = BACKBUFFER_WIDTH;
	swap_chain.BufferDesc.Height = BACKBUFFER_HEIGHT;
	swap_chain.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swap_chain.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swap_chain.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swap_chain.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain.SampleDesc.Count = 1;
	swap_chain.OutputWindow = window;
	swap_chain.Windowed = true;
	swap_chain.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swap_chain.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; //alt+enter fullscreen

	D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG, NULL, 0, D3D11_SDK_VERSION, &swap_chain, &swapchain, &device, NULL, &deviceContext);

	CComPtr<ID3D11Resource> resource;

	swapchain->GetBuffer(0, __uuidof(resource),
		reinterpret_cast<void**>(&resource));

	device->CreateRenderTargetView(resource, NULL, &rtv);

	resource.p->Release();

#pragma endregion

#pragma region Viewport
	// Viewport description
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = BACKBUFFER_WIDTH;
	viewport.Height = BACKBUFFER_HEIGHT;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;
#pragma endregion

#pragma region Vertex buffers
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

	D3D11_BUFFER_DESC skeleton_vbuffer;
	ZeroMemory(&skeleton_vbuffer, sizeof(D3D11_BUFFER_DESC));
	skeleton_vbuffer.ByteWidth = sizeof(Importer::Skeleton::joints[0].animation->globalTransform) * import.skeleton.joints.size();
	skeleton_vbuffer.Usage = D3D11_USAGE_IMMUTABLE;
	skeleton_vbuffer.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	skeleton_vbuffer.StructureByteStride = sizeof(Importer::Skeleton::joints[0].animation->globalTransform);

	D3D11_SUBRESOURCE_DATA data_2;
	ZeroMemory(&data_2, sizeof(D3D11_SUBRESOURCE_DATA));
	data_2.pSysMem = import.skeleton.joints.data();

	device->CreateBuffer(&skeleton_vbuffer, &data_2, &vertexBufferSkeleton.p);
	
#pragma endregion

#pragma region Index buffers
	// Index buffer
	D3D11_BUFFER_DESC model_ibuffer;
	ZeroMemory(&model_ibuffer, sizeof(D3D11_BUFFER_DESC));
	model_ibuffer.Usage = D3D11_USAGE_IMMUTABLE;
	model_ibuffer.ByteWidth = sizeof(UINT) * import.uniqueIndicies.size();
	model_ibuffer.BindFlags = D3D11_BIND_INDEX_BUFFER;
	model_ibuffer.StructureByteStride = sizeof(UINT);

	D3D11_SUBRESOURCE_DATA idata;
	ZeroMemory(&idata, sizeof(D3D11_SUBRESOURCE_DATA));
	idata.pSysMem = import.uniqueIndicies.data();

	device->CreateBuffer(&model_ibuffer, &idata, &indexBuffer);

	D3D11_BUFFER_DESC skeleton_ibuffer;
	ZeroMemory(&skeleton_ibuffer, sizeof(D3D11_BUFFER_DESC));
	skeleton_ibuffer.Usage = D3D11_USAGE_IMMUTABLE;
	skeleton_ibuffer.ByteWidth = sizeof(UINT) * import.skeleton.joints.size();
	skeleton_ibuffer.BindFlags = D3D11_BIND_INDEX_BUFFER;
	skeleton_ibuffer.StructureByteStride = sizeof(UINT);

	D3D11_SUBRESOURCE_DATA idata1;
	ZeroMemory(&idata1, sizeof(D3D11_SUBRESOURCE_DATA));
	idata1.pSysMem = import.skeleton.joints.data();

	device->CreateBuffer(&skeleton_ibuffer, &idata1, &indexBufferSkeleton);

#pragma endregion

#pragma region Depth Stencil
	D3D11_TEXTURE2D_DESC model_depth;
	ZeroMemory(&model_depth, sizeof(D3D11_TEXTURE2D_DESC));

	model_depth.Width = BACKBUFFER_WIDTH;
	model_depth.Height = BACKBUFFER_HEIGHT;
	model_depth.MipLevels = 1;
	model_depth.ArraySize = 1;
	model_depth.Format = DXGI_FORMAT_D32_FLOAT;
	model_depth.SampleDesc.Count = 1;
	model_depth.Usage = D3D11_USAGE_DEFAULT;
	model_depth.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	device->CreateTexture2D(&model_depth, NULL, &depthStencil.p);

	D3D11_DEPTH_STENCIL_VIEW_DESC model_dsvd;
	ZeroMemory(&model_dsvd, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	model_dsvd.Format = DXGI_FORMAT_D32_FLOAT;
	model_dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	model_dsvd.Texture2D.MipSlice = 0;

	device->CreateDepthStencilView(depthStencil, &model_dsvd, &dsView.p);

	deviceContext->OMSetRenderTargets(1, &rtv.p, dsView);
#pragma endregion

#pragma region Shaders
	device->CreateVertexShader(RTA_VS, sizeof(RTA_VS), nullptr, &vShader.p);
	device->CreatePixelShader(RTA_PS, sizeof(RTA_PS), nullptr, &pShader.p);

	D3D11_INPUT_ELEMENT_DESC model_Layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMALS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	device->CreateInputLayout(model_Layout, ARRAYSIZE(model_Layout), RTA_VS, sizeof(RTA_VS), &input.p);
#pragma endregion

#pragma region Constant Buffers
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

	D3D11_BUFFER_DESC cbufferDirectional;
	ZeroMemory(&cbufferDirectional, sizeof(D3D11_BUFFER_DESC));

	cbufferDirectional.ByteWidth = sizeof(DIRECTIONAL_LIGHT);
	cbufferDirectional.Usage = D3D11_USAGE_DYNAMIC;
	cbufferDirectional.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbufferDirectional.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbufferDirectional.StructureByteStride = sizeof(float);

	device->CreateBuffer(&cbufferDirectional, NULL, &constantBufferDirectional.p);

	D3D11_BUFFER_DESC cbufferSkeleton;
	ZeroMemory(&cbufferSkeleton, sizeof(D3D11_BUFFER_DESC));

	cbufferSkeleton.ByteWidth = sizeof(FbxAMatrix);
	cbufferSkeleton.Usage = D3D11_USAGE_DYNAMIC;
	cbufferSkeleton.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbufferSkeleton.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbufferSkeleton.StructureByteStride = sizeof(double);

	device->CreateBuffer(&cbufferSkeleton, NULL, &constantBufferSkeleton.p);

#pragma endregion

#pragma region Sampler
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));

	samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_GREATER;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&samplerDesc, &sampler.p);
#pragma endregion

#pragma region Camera
	model.worldMatrix = DirectX::XMMatrixTranslation(0.0f, 0.0f, 3.0f);
	bones.worldMatrix = DirectX::XMMatrixTranslation(5.0f, 0.0f, 3.0f);

	camera.viewMatrix = DirectX::XMMatrixIdentity();
	camera.viewMatrix = XMMatrixInverse(NULL, camera.viewMatrix);
	camera.projMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(65), aspectRatio, zNear, zFar);

	// Direction light
	direction.directionalMatrix = DirectX::XMMatrixIdentity();
	direction.directionaldir = { 0.0f, 0.0f, 1.0f, 0.0f };
	direction.directionalcolor = { 1.0f, 1.0f, 0.878f, 1.0f };
	direction.directionalMatrix.r[3].m128_f32[0] = direction.directionaldir.x;
	direction.directionalMatrix.r[3].m128_f32[1] = direction.directionaldir.y;
	direction.directionalMatrix.r[3].m128_f32[2] = direction.directionaldir.z;
	direction.directionalMatrix.r[3].m128_f32[3] = direction.directionaldir.w;

#pragma endregion

#pragma region Texture DDS
	CreateDDSTextureFromFile(device, L"Teddy_D.dds", NULL, &srv.p);
#pragma endregion

}

bool RTA_PROJECT::Run()
{
	timer.Signal();

	Camera_Movement();
	Sun();
	//GetCursorPos(&currPos);

	deviceContext->ClearRenderTargetView(rtv, clearColor);
	deviceContext->ClearDepthStencilView(dsView, D3D11_CLEAR_DEPTH, 1, 0);

	deviceContext->VSSetConstantBuffers(0, 1, &constantBufferObj.p);
	deviceContext->VSSetConstantBuffers(1, 1, &constantBufferScene.p);
	//deviceContext->VSSetConstantBuffers(2, 1, &constantBufferSkeleton.p);

	deviceContext->PSSetConstantBuffers(0, 1, &constantBufferDirectional.p);

	D3D11_MAPPED_SUBRESOURCE directionalLight;
	deviceContext->Map(constantBufferDirectional, 0, D3D11_MAP_WRITE_DISCARD, 0, &directionalLight);
	memcpy(directionalLight.pData, &direction, sizeof(DIRECTIONAL_LIGHT));
	deviceContext->Unmap(constantBufferDirectional, 0);

	D3D11_MAPPED_SUBRESOURCE map_camera;
	deviceContext->Map(constantBufferScene, 0, D3D11_MAP_WRITE_DISCARD, 0, &map_camera);
	memcpy(map_camera.pData, &camera, sizeof(SCENE));
	deviceContext->Unmap(constantBufferScene, 0);

	deviceContext->RSSetViewports(1, &viewport);

#pragma region Teddy
	D3D11_MAPPED_SUBRESOURCE map;
	deviceContext->Map(constantBufferObj, 0, D3D11_MAP_WRITE_DISCARD, 0, &map);
	memcpy(map.pData, &model, sizeof(OBJECT));
	deviceContext->Unmap(constantBufferObj, 0);

	UINT stridesize = sizeof(Importer::Vertex);
	UINT strideoffset = 0;
	deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer.p, &stridesize, &strideoffset);
	deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	deviceContext->PSSetShaderResources(0, 1, &srv.p);

	deviceContext->VSSetShader(vShader, nullptr, 0);
	deviceContext->PSSetShader(pShader, nullptr, 0);

	deviceContext->IASetInputLayout(input);
	
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	
	deviceContext->DrawIndexed(import.uniqueIndicies.size(), 0, 0);
	//deviceContext->Draw(import.totalVertexes.size(), 0);
#pragma endregion

#pragma region Skeleton
	/*D3D11_MAPPED_SUBRESOURCE map1;
	deviceContext->Map(constantBufferSkeleton, 0, D3D11_MAP_WRITE_DISCARD, 0, &map1);
	memcpy(map1.pData, &bones, sizeof(OBJECT));
	deviceContext->Unmap(constantBufferSkeleton, 0);

	UINT strideSize1 = sizeof(import.skeleton.joints[0].animation->globalTransform);
	UINT strideOffset1 = 0;
	deviceContext->IASetVertexBuffers(0, 1, &vertexBufferSkeleton, &strideSize1, &strideOffset1);
	deviceContext->IASetIndexBuffer(indexBufferSkeleton, DXGI_FORMAT_R32_UINT, 0);

	ID3D11ShaderResourceView* nullsrv = nullptr;

	deviceContext->VSSetShader(vShader, nullptr, 0);
	deviceContext->PSSetShader(pShader, nullptr, 0);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	deviceContext->Draw(37, 0);*/
#pragma endregion
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

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
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

void RTA_PROJECT::Camera_Movement()
{
	

	camera.viewMatrix = XMMatrixInverse(NULL, camera.viewMatrix);

	if (GetAsyncKeyState('W'))
	{
		DirectX::XMVECTOR temp;
		temp.m128_f32[0] = 0;
		temp.m128_f32[1] = 0;
		temp.m128_f32[2] = 0.005f;
		temp.m128_f32[3] = camera.viewMatrix.r[3].m128_f32[3];

		camera.viewMatrix = XMMatrixMultiply(DirectX::XMMatrixTranslationFromVector(temp), camera.viewMatrix);
	}

	if (GetAsyncKeyState('S'))
	{
		DirectX::XMVECTOR temp;
		temp.m128_f32[0] = 0;
		temp.m128_f32[1] = 0;
		temp.m128_f32[2] = -0.005f;
		temp.m128_f32[3] = camera.viewMatrix.r[3].m128_f32[3];

		camera.viewMatrix = XMMatrixMultiply(DirectX::XMMatrixTranslationFromVector(temp), camera.viewMatrix);
	}

	if (GetAsyncKeyState('A'))
	{
		DirectX::XMVECTOR temp;
		temp.m128_f32[0] = -0.005f;
		temp.m128_f32[1] = 0;
		temp.m128_f32[2] = 0;
		temp.m128_f32[3] = camera.viewMatrix.r[3].m128_f32[3];

		camera.viewMatrix = XMMatrixMultiply(DirectX::XMMatrixTranslationFromVector(temp), camera.viewMatrix);
	}

	if (GetAsyncKeyState('D'))
	{
		DirectX::XMVECTOR temp;
		temp.m128_f32[0] = 0.005f;
		temp.m128_f32[1] = 0;
		temp.m128_f32[2] = 0;
		temp.m128_f32[3] = camera.viewMatrix.r[3].m128_f32[3];

		camera.viewMatrix = XMMatrixMultiply(DirectX::XMMatrixTranslationFromVector(temp), camera.viewMatrix);
	}

	if (GetAsyncKeyState('Z'))
	{
		camera.viewMatrix.r[3].m128_f32[1] += 0.005f;
	}

	if (GetAsyncKeyState('X'))
	{
		camera.viewMatrix.r[3].m128_f32[1] -= 0.005f;
	}

	if (GetAsyncKeyState(VK_UP))
	{
		DirectX::XMVECTOR temp;

		temp.m128_f32[0] = camera.viewMatrix.r[3].m128_f32[0];
		temp.m128_f32[1] = camera.viewMatrix.r[3].m128_f32[1];
		temp.m128_f32[2] = camera.viewMatrix.r[3].m128_f32[2];

		camera.viewMatrix.r[3].m128_f32[0] = camera.viewMatrix.r[3].m128_f32[1] = camera.viewMatrix.r[3].m128_f32[2] = 0;

		camera.viewMatrix = XMMatrixMultiply(DirectX::XMMatrixRotationX(-0.005f), camera.viewMatrix);

		camera.viewMatrix.r[3].m128_f32[0] = temp.m128_f32[0];
		camera.viewMatrix.r[3].m128_f32[1] = temp.m128_f32[1];
		camera.viewMatrix.r[3].m128_f32[2] = temp.m128_f32[2];
	}

	if (GetAsyncKeyState(VK_DOWN))
	{
		DirectX::XMVECTOR temp;

		temp.m128_f32[0] = camera.viewMatrix.r[3].m128_f32[0];
		temp.m128_f32[1] = camera.viewMatrix.r[3].m128_f32[1];
		temp.m128_f32[2] = camera.viewMatrix.r[3].m128_f32[2];

		camera.viewMatrix.r[3].m128_f32[0] = camera.viewMatrix.r[3].m128_f32[1] = camera.viewMatrix.r[3].m128_f32[2] = 0;

		camera.viewMatrix = XMMatrixMultiply(DirectX::XMMatrixRotationX(0.005f), camera.viewMatrix);

		camera.viewMatrix.r[3].m128_f32[0] = temp.m128_f32[0];
		camera.viewMatrix.r[3].m128_f32[1] = temp.m128_f32[1];
		camera.viewMatrix.r[3].m128_f32[2] = temp.m128_f32[2];
	}

	if (GetAsyncKeyState(VK_RIGHT))
	{
		DirectX::XMVECTOR temp;

		temp.m128_f32[0] = camera.viewMatrix.r[3].m128_f32[0];
		temp.m128_f32[1] = camera.viewMatrix.r[3].m128_f32[1];
		temp.m128_f32[2] = camera.viewMatrix.r[3].m128_f32[2];

		camera.viewMatrix.r[3].m128_f32[0] = camera.viewMatrix.r[3].m128_f32[1] = camera.viewMatrix.r[3].m128_f32[2] = 0;

		camera.viewMatrix = XMMatrixMultiply(camera.viewMatrix, DirectX::XMMatrixRotationY(0.005f));

		camera.viewMatrix.r[3].m128_f32[0] = temp.m128_f32[0];
		camera.viewMatrix.r[3].m128_f32[1] = temp.m128_f32[1];
		camera.viewMatrix.r[3].m128_f32[2] = temp.m128_f32[2];
	}

	if (GetAsyncKeyState(VK_LEFT))
	{
		DirectX::XMVECTOR temp;

		temp.m128_f32[0] = camera.viewMatrix.r[3].m128_f32[0];
		temp.m128_f32[1] = camera.viewMatrix.r[3].m128_f32[1];
		temp.m128_f32[2] = camera.viewMatrix.r[3].m128_f32[2];

		camera.viewMatrix.r[3].m128_f32[0] = camera.viewMatrix.r[3].m128_f32[1] = camera.viewMatrix.r[3].m128_f32[2] = 0;

		camera.viewMatrix = XMMatrixMultiply(camera.viewMatrix, DirectX::XMMatrixRotationY(-0.005f));

		camera.viewMatrix.r[3].m128_f32[0] = temp.m128_f32[0];
		camera.viewMatrix.r[3].m128_f32[1] = temp.m128_f32[1];
		camera.viewMatrix.r[3].m128_f32[2] = temp.m128_f32[2];
	}
	/*if (GetAsyncKeyState(VK_RBUTTON))
	{

		POINT tempMouse;
		GetCursorPos(&tempMouse);
		float deltaX = tempMouse.x - currPos.x;
		float deltaY = tempMouse.y - currPos.y;

		DirectX::XMVECTOR tempOs = { camera.viewMatrix.r[3].m128_f32[0],camera.viewMatrix.r[3].m128_f32[1], camera.viewMatrix.r[3].m128_f32[2], camera.viewMatrix.r[3].m128_f32[3] };

		camera.viewMatrix.r[3].m128_f32[0] = 0;
		camera.viewMatrix.r[3].m128_f32[1] = 0;
		camera.viewMatrix.r[3].m128_f32[2] = 0;

		camera.viewMatrix = DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationX(deltaY * (10 * timer.Delta())), camera.viewMatrix);
		camera.viewMatrix = DirectX::XMMatrixMultiply(camera.viewMatrix, DirectX::XMMatrixRotationY(deltaX * (10 * timer.Delta())));

		camera.viewMatrix.r[3].m128_f32[0] = tempOs.m128_f32[0];
		camera.viewMatrix.r[3].m128_f32[1] = tempOs.m128_f32[1];
		camera.viewMatrix.r[3].m128_f32[2] = tempOs.m128_f32[2];

		currPos = tempMouse;

	}*/
	camera.viewMatrix = XMMatrixInverse(NULL, camera.viewMatrix);
}

void RTA_PROJECT::Sun()
{
	// Directional movement
	DirectX::XMMATRIX sunmovement = DirectX::XMMatrixIdentity();

	sunmovement.r[3].m128_f32[0] = direction.directionaldir.x;
	sunmovement.r[3].m128_f32[1] = direction.directionaldir.y;
	sunmovement.r[3].m128_f32[2] = direction.directionaldir.z;
	sunmovement.r[3].m128_f32[3] = direction.directionaldir.w;

	sunmovement = XMMatrixMultiply(sunmovement, DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians((float)timer.Delta() * 20)));

	direction.directionaldir.x = sunmovement.r[3].m128_f32[0];
	direction.directionaldir.y = sunmovement.r[3].m128_f32[1];
	direction.directionaldir.z = sunmovement.r[3].m128_f32[2];
	direction.directionaldir.w = sunmovement.r[3].m128_f32[3];
}

int main(int args, char**argv)
{

}