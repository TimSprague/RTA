#include "stdafx.h"
#include "Importer.h"
#include "Interpolator.h"

float aspectRatio = (float)(BACKBUFFER_WIDTH) / (BACKBUFFER_HEIGHT);
float zNear = 0.1f;
float zFar = 5000.0f;
float zBuffer[PIXELS];
UINT Raster[PIXELS];

float clearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };

bool modelswitch = true;

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

	CComPtr<ID3D11Buffer> vertexBuffer, vertexBufferLine, vertexBufferCube, indexBuffer, indexBufferLine, indexBufferCube, indexBufferSkybox, constantBufferObj, constantBufferScene, constantBufferDirectional, constantBufferCube, constantBufferLine, constantBufferInstance, constantBufferInstanceLine, vertexBufferSkybox;

	CComPtr<ID3D11InputLayout> input, input1, input2;

	CComPtr<ID3D11VertexShader> vShader, vShader1, vShader2, vShaderInstance, vShaderInstanceLines;

	CComPtr<ID3D11PixelShader> pShader, pShader1, pShader2;

	CComPtr<ID3D11DepthStencilView> dsView;

	CComPtr<ID3D11Texture2D> depthStencil;

	CComPtr<ID3D11ShaderResourceView> srv, srv1;

	CComPtr<ID3D11SamplerState> sampler;

	CComPtr<ID3D11RasterizerState> rasterstate, rasterstate1;

	CComPtr<ID3D11Resource> texture;

	OBJECT model, cube, skybox_1;

	SCENE camera;

	Importer import;

	Interpolator interpolate;

	DIRECTIONAL_LIGHT direction;

	LINE line;

	POINT currPos;

	XTime timer;

	DirectX::XMMATRIX temp = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX temp1 = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX temp_lines = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX clones[36];

	UINT line_points[36];

	int frame = 0;
	double timeCheck = 0;


public:

	RTA_PROJECT(HINSTANCE hinst, WNDPROC proc);
	bool Run();
	bool ShutDown();
	void Camera_Movement();
	void Sun();
	DirectX::XMMATRIX ConvertFBX_DX11(FbxAMatrix _m);
	void ANIMATE(int _frame);
	//void Skeleton(int _frame);

};


RTA_PROJECT::RTA_PROJECT(HINSTANCE hinst, WNDPROC proc)
{
	import.ImportFile("Teddy_Idle.fbx");
	
#pragma region Window
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
#pragma endregion

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

#pragma region Cube

	CUBE square[8];

	square[0].pos.m128_f32[0] = -1.0f;
	square[0].pos.m128_f32[1] = 1.0f;
	square[0].pos.m128_f32[2] = -1.0f;
	square[0].pos.m128_f32[3] = 1.0f;

	square[1].pos.m128_f32[0] = 1.0f;
	square[1].pos.m128_f32[1] = 1.0f;
	square[1].pos.m128_f32[2] = -1.0f;
	square[1].pos.m128_f32[3] = 1.0f;
	
	square[2].pos.m128_f32[0] = -1.0f;
	square[2].pos.m128_f32[1] = -1.0f;
	square[2].pos.m128_f32[2] = -1.0f;
	square[2].pos.m128_f32[3] = 1.0f;

	square[3].pos.m128_f32[0] = 1.0f;
	square[3].pos.m128_f32[1] = -1.0f;
	square[3].pos.m128_f32[2] = -1.0f;
	square[3].pos.m128_f32[3] = 1.0f;

	square[4].pos.m128_f32[0] = -1.0f;
	square[4].pos.m128_f32[1] = 1.0f;
	square[4].pos.m128_f32[2] = 1.0f;
	square[4].pos.m128_f32[3] = 1.0f;

	square[5].pos.m128_f32[0] = 1.0f;
	square[5].pos.m128_f32[1] = 1.0f;
	square[5].pos.m128_f32[2] = 1.0f;
	square[5].pos.m128_f32[3] = 1.0f;

	square[6].pos.m128_f32[0] = -1.0f;
	square[6].pos.m128_f32[1] = -1.0f;
	square[6].pos.m128_f32[2] = 1.0f;
	square[6].pos.m128_f32[3] = 1.0f;

	square[7].pos.m128_f32[0] = 1.0f;
	square[7].pos.m128_f32[1] = -1.0f;
	square[7].pos.m128_f32[2] = 1.0f;
	square[7].pos.m128_f32[3] = 1.0f;

	UINT cube_indicies[36] =
	{
		0,1,2,	// side 1
		2,1,3,
		4,0,6,	// side 2
		6,0,2, 
		7,5,6,	// side 3
		6,5,4,
		3,1,7,	// side 4
		7,1,5,
		4,5,0,	// side 5
		0,5,1,
		3,7,2,	// side 6
		2,7,6,
	};



	

	
#pragma endregion

#pragma region Lines

	// Array of lines for the vertex buffer
	LINE arr_lines[36];
	ZeroMemory(arr_lines, sizeof(arr_lines));
	for (UINT i = 0; i < ARRAYSIZE(arr_lines); ++i)
	{
		arr_lines[i].pos = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		arr_lines[i].rgba = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	}

	// Indicies of the 
	UINT line_indicies[62] =
	{
		0,1,1,2,
		2,3,3,4,
		3,6,6,7,
		7,8,8,9,
		9,10,9,11,
		9,12,3,13,
		13,14,14,15,
		15,16,16,17,
		16,18,16,19,
		3,21,21,22,
		22,23,23,24,
		24,25,0,26,
		26,27,27,28,
		28,29,0,31,
		31,32,32,33,
		33,34
	};
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

	D3D11_BUFFER_DESC cube_vbuffer;
	ZeroMemory(&cube_vbuffer, sizeof(D3D11_BUFFER_DESC));
	cube_vbuffer.ByteWidth = sizeof(square);
	cube_vbuffer.Usage = D3D11_USAGE_IMMUTABLE;
	cube_vbuffer.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	cube_vbuffer.StructureByteStride = sizeof(CUBE);

	D3D11_SUBRESOURCE_DATA data_2;
	ZeroMemory(&data_2, sizeof(D3D11_SUBRESOURCE_DATA));
	data_2.pSysMem = square;

	device->CreateBuffer(&cube_vbuffer, &data_2, &vertexBufferCube.p);

	// Line
	D3D11_BUFFER_DESC line_vbuffer;
	ZeroMemory(&line_vbuffer, sizeof(D3D11_BUFFER_DESC));
	line_vbuffer.ByteWidth = sizeof(LINE) * 36;
	line_vbuffer.Usage = D3D11_USAGE_IMMUTABLE;
	line_vbuffer.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	line_vbuffer.StructureByteStride = sizeof(LINE);

	D3D11_SUBRESOURCE_DATA data_3;
	ZeroMemory(&data_3, sizeof(D3D11_SUBRESOURCE_DATA));
	data_3.pSysMem = arr_lines;

	device->CreateBuffer(&line_vbuffer, &data_3, &vertexBufferLine.p);

	// Vertex buffer for skybox
	D3D11_BUFFER_DESC vbufferSkybox;
	ZeroMemory(&vbufferSkybox, sizeof(D3D11_BUFFER_DESC));

	vbufferSkybox.ByteWidth = sizeof(square);
	vbufferSkybox.Usage = D3D11_USAGE_IMMUTABLE;
	vbufferSkybox.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbufferSkybox.StructureByteStride = sizeof(OBJVERTEX);

	D3D11_SUBRESOURCE_DATA skyboxData;
	ZeroMemory(&skyboxData, sizeof(D3D11_SUBRESOURCE_DATA));
	skyboxData.pSysMem = square;

	device->CreateBuffer(&vbufferSkybox, &skyboxData, &vertexBufferSkybox.p);
	
#pragma endregion

#pragma region Index buffers

	// Index buffer for Teddy
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

	// Index buffer for cubes
	D3D11_BUFFER_DESC cube_ibuffer;
	ZeroMemory(&cube_ibuffer, sizeof(D3D11_BUFFER_DESC));
	cube_ibuffer.Usage = D3D11_USAGE_IMMUTABLE;
	cube_ibuffer.ByteWidth = sizeof(cube_indicies);
	cube_ibuffer.BindFlags = D3D11_BIND_INDEX_BUFFER;
	cube_ibuffer.StructureByteStride = sizeof(UINT);

	D3D11_SUBRESOURCE_DATA idata1;
	ZeroMemory(&idata1, sizeof(D3D11_SUBRESOURCE_DATA));
	idata1.pSysMem = cube_indicies;

	device->CreateBuffer(&cube_ibuffer, &idata1, &indexBufferCube.p);

	// Index buffer for Lines
	D3D11_BUFFER_DESC line_ibuffer;
	ZeroMemory(&line_ibuffer, sizeof(D3D11_BUFFER_DESC));
	line_ibuffer.Usage = D3D11_USAGE_IMMUTABLE;
	line_ibuffer.ByteWidth = sizeof(line_indicies);
	line_ibuffer.BindFlags = D3D11_BIND_INDEX_BUFFER;
	line_ibuffer.StructureByteStride = sizeof(UINT);
	
	D3D11_SUBRESOURCE_DATA idata2;
	ZeroMemory(&idata2, sizeof(D3D11_SUBRESOURCE_DATA));
	idata2.pSysMem = line_indicies;

	device->CreateBuffer(&line_ibuffer, &idata2, &indexBufferLine.p);

	// Index buffer for Skybox
	D3D11_BUFFER_DESC ibufferSkybox;
	ZeroMemory(&ibufferSkybox, sizeof(D3D11_BUFFER_DESC));
	ibufferSkybox.Usage = D3D11_USAGE_IMMUTABLE;
	ibufferSkybox.ByteWidth = sizeof(cube_indicies);
	ibufferSkybox.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibufferSkybox.StructureByteStride = sizeof(UINT);

	D3D11_SUBRESOURCE_DATA iDataSkybox;
	ZeroMemory(&iDataSkybox, sizeof(D3D11_SUBRESOURCE_DATA));
	iDataSkybox.pSysMem = cube_indicies;

	device->CreateBuffer(&ibufferSkybox, &iDataSkybox, &indexBufferSkybox.p);

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

#pragma region Raster buffers
	
	// Raster buffers for skybox

	D3D11_RASTERIZER_DESC skyboxRaster;
	ZeroMemory(&skyboxRaster, sizeof(D3D11_RASTERIZER_DESC));

	skyboxRaster.DepthClipEnable = TRUE;
	skyboxRaster.FrontCounterClockwise = TRUE;
	skyboxRaster.FillMode = D3D11_FILL_SOLID;
	skyboxRaster.CullMode = D3D11_CULL_BACK;

	device->CreateRasterizerState(&skyboxRaster, &rasterstate.p);

	D3D11_RASTERIZER_DESC skyboxRaster1;
	ZeroMemory(&skyboxRaster1, sizeof(D3D11_RASTERIZER_DESC));

	skyboxRaster1.FrontCounterClockwise = FALSE;
	skyboxRaster1.DepthClipEnable = TRUE;
	skyboxRaster1.FillMode = D3D11_FILL_SOLID;
	skyboxRaster1.CullMode = D3D11_CULL_BACK;

	device->CreateRasterizerState(&skyboxRaster1, &rasterstate1.p);

#pragma endregion

#pragma region Shaders

	// Teddy model
	device->CreateVertexShader(RTA_VS, sizeof(RTA_VS), nullptr, &vShader.p);
	device->CreatePixelShader(RTA_PS, sizeof(RTA_PS), nullptr, &pShader.p);

	// Boxes
	device->CreateVertexShader(Instance_VS, sizeof(Instance_VS), nullptr, &vShaderInstance.p);
	device->CreateVertexShader(Instance_lines_VS, sizeof(Instance_lines_VS), nullptr, &vShaderInstanceLines.p);

	// Lines
	device->CreateVertexShader(Lines_VS, sizeof(Lines_VS), nullptr, &vShader1.p);
	device->CreatePixelShader(Lines_PS, sizeof(Lines_PS), nullptr, &pShader1.p);

	// Skybox
	device->CreateVertexShader(Skybox_VS, sizeof(Skybox_VS), nullptr, &vShader2.p);
	device->CreatePixelShader(Skybox_PS, sizeof(Skybox_PS), nullptr, &pShader2.p);


	D3D11_INPUT_ELEMENT_DESC model_Layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMALS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	device->CreateInputLayout(model_Layout, ARRAYSIZE(model_Layout), RTA_VS, sizeof(RTA_VS), &input.p);

	D3D11_INPUT_ELEMENT_DESC line_Layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	device->CreateInputLayout(line_Layout, ARRAYSIZE(line_Layout), Lines_VS, sizeof(Lines_VS), &input1.p);

	D3D11_INPUT_ELEMENT_DESC skyboxLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	device->CreateInputLayout(skyboxLayout, ARRAYSIZE(skyboxLayout), Skybox_VS, sizeof(Skybox_VS), &input2.p);

#pragma endregion

#pragma region Constant Buffers
	
	// Constant buffer for Teddy or any other object
	D3D11_BUFFER_DESC cbufferObject;
	ZeroMemory(&cbufferObject, sizeof(D3D11_BUFFER_DESC));

	cbufferObject.ByteWidth = sizeof(OBJECT);
	cbufferObject.Usage = D3D11_USAGE_DYNAMIC;
	cbufferObject.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbufferObject.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbufferObject.StructureByteStride = sizeof(float);

	device->CreateBuffer(&cbufferObject, NULL, &constantBufferObj.p);

	// Constant buffer for camera
	D3D11_BUFFER_DESC cbufferScene;
	ZeroMemory(&cbufferScene, sizeof(D3D11_BUFFER_DESC));

	cbufferScene.ByteWidth = sizeof(SCENE);
	cbufferScene.Usage = D3D11_USAGE_DYNAMIC;
	cbufferScene.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbufferScene.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbufferScene.StructureByteStride = sizeof(float);

	device->CreateBuffer(&cbufferScene, NULL, &constantBufferScene.p);

	// Constant buffer for directional light
	D3D11_BUFFER_DESC cbufferDirectional;
	ZeroMemory(&cbufferDirectional, sizeof(D3D11_BUFFER_DESC));

	cbufferDirectional.ByteWidth = sizeof(DIRECTIONAL_LIGHT);
	cbufferDirectional.Usage = D3D11_USAGE_DYNAMIC;
	cbufferDirectional.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbufferDirectional.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbufferDirectional.StructureByteStride = sizeof(float);

	device->CreateBuffer(&cbufferDirectional, NULL, &constantBufferDirectional.p);

	// Constant buffer for instancing 
	D3D11_BUFFER_DESC cbufferInstance;
	ZeroMemory(&cbufferInstance, sizeof(D3D11_BUFFER_DESC));

	cbufferInstance.ByteWidth = sizeof(clones);
	cbufferInstance.Usage = D3D11_USAGE_DYNAMIC;
	cbufferInstance.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbufferInstance.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbufferInstance.StructureByteStride = sizeof(DirectX::XMMATRIX);

	device->CreateBuffer(&cbufferInstance, NULL, &constantBufferInstance.p);

	D3D11_BUFFER_DESC cbufferInstanceLines;
	ZeroMemory(&cbufferInstanceLines, sizeof(D3D11_BUFFER_DESC));

	cbufferInstanceLines.ByteWidth = sizeof(clones);
	cbufferInstanceLines.Usage = D3D11_USAGE_DYNAMIC;
	cbufferInstanceLines.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbufferInstanceLines.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbufferInstanceLines.StructureByteStride = sizeof(DirectX::XMMATRIX);

	device->CreateBuffer(&cbufferInstanceLines, NULL, &constantBufferInstanceLine.p);

#pragma endregion

#pragma region Sampler

	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));

	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_GREATER;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&samplerDesc, &sampler.p);

#pragma endregion

#pragma region Camera

	model.worldMatrix = DirectX::XMMatrixTranslation(0.0f, 0.0f, 75.0f);

	//for (UINT i = 0; i < jointsCount; i++)
	//{
	//	if (!import.skeleton.joints[i].animation)
	//	{
	//		clones[i] = DirectX::XMMatrixIdentity();
	//		continue;
	//	}
	//	clones[i] = ConvertFBX_DX11(import.skeleton.joints[i].animation->globalTransform);
	//	temp = clones[i];
	//	clones[i] = cube.worldMatrix = DirectX::XMMatrixTranslation(temp.r[3].m128_f32[0], temp.r[3].m128_f32[1], temp.r[3].m128_f32[2]);
	//}

#pragma region Lines

	line.rgba = { 1.0f, 1.0f, 1.0f, 1.0f };

#pragma endregion

	camera.viewMatrix = DirectX::XMMatrixIdentity();
	camera.viewMatrix = DirectX::XMMatrixInverse(NULL, camera.viewMatrix);
	camera.projMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(65), aspectRatio, zNear, zFar);

	// Direction light
	direction.directionalMatrix = DirectX::XMMatrixIdentity();
	direction.directionaldir = { 0.0f, 0.0f, 1.0f, 0.0f };
	direction.directionalcolor = { 1.0f, 1.0f, 0.878f, 1.0f };
	direction.directionalMatrix.r[3].m128_f32[0] = direction.directionaldir.x;
	direction.directionalMatrix.r[3].m128_f32[1] = direction.directionaldir.y;
	direction.directionalMatrix.r[3].m128_f32[2] = direction.directionaldir.z;
	direction.directionalMatrix.r[3].m128_f32[3] = direction.directionaldir.w;

	skybox_1.worldMatrix = DirectX::XMMatrixIdentity();
	skybox_1.worldMatrix = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);

#pragma endregion

#pragma region Texture DDS

	CreateDDSTextureFromFile(device, L"Teddy_D.dds", NULL, &srv.p);
	CreateDDSTextureFromFile(device, L"sahara.dds", NULL, &srv1.p);

#pragma endregion

}

void RTA_PROJECT::ANIMATE(int _frame)
{

		for (unsigned int jointIndex = 0; jointIndex < import.skeleton.joints.size(); jointIndex++)
		{
			// 5 30 35
			if (jointIndex == 5 || jointIndex == 30 || jointIndex == 35 || jointIndex == 20)
			{
				continue;
			}
			import.skeleton.joints[jointIndex].animation[_frame].globalTransform;
			clones[jointIndex] = ConvertFBX_DX11(import.skeleton.joints[jointIndex].animation[_frame].globalTransform);
			temp = clones[jointIndex];
			clones[jointIndex] = cube.worldMatrix = DirectX::XMMatrixTranslation(temp.r[3].m128_f32[0], temp.r[3].m128_f32[1], temp.r[3].m128_f32[2]);
		}

		int my_d = INT_MAX;

}

//void RTA_PROJECT::Skeleton(int _frame)
//{
//	for (unsigned int jointIndex = 0; jointIndex < import.skeleton.joints.size(); jointIndex++)
//	{
//		// 5 30 35
//		if (jointIndex == 5 || jointIndex == 30 || jointIndex == 35 || jointIndex == 20)
//		{
//			continue;
//		}
//		import.skeleton.joints[jointIndex].animation[_frame].globalTransform;
//		clones[jointIndex] = ConvertFBX_DX11(import.skeleton.joints[jointIndex].animation[_frame].globalTransform);
//		temp = clones[jointIndex];
//		clones[jointIndex] = line.worldMatrix = DirectX::XMMatrixTranslation(temp.r[3].m128_f32[0], temp.r[3].m128_f32[1], temp.r[3].m128_f32[2]);
//	}
//}

bool RTA_PROJECT::Run()
{
	if (frame >= 60)
	{
		frame = 1;
	}

	timer.Signal();
	timeCheck += timer.SmoothDelta();

	Camera_Movement();
	//Sun();

	if (timeCheck >= (20.f/60.f))
	{
		timeCheck = 0;
		ANIMATE(frame);
	}
	//GetCursorPos(&currPos);

	//interpolate.Process(&import.mAnimation);

	deviceContext->ClearRenderTargetView(rtv, clearColor);
	deviceContext->ClearDepthStencilView(dsView, D3D11_CLEAR_DEPTH, 1, 0);

	deviceContext->VSSetConstantBuffers(0, 1, &constantBufferObj.p);
	deviceContext->VSSetConstantBuffers(1, 1, &constantBufferScene.p);
	deviceContext->VSSetConstantBuffers(2, 1, &constantBufferInstance.p);
	deviceContext->VSSetConstantBuffers(3, 1, &constantBufferInstanceLine.p);

	deviceContext->PSSetConstantBuffers(0, 1, &constantBufferDirectional.p);

	deviceContext->PSSetSamplers(0, 1, &sampler.p);

	D3D11_MAPPED_SUBRESOURCE directionalLight;
	deviceContext->Map(constantBufferDirectional, 0, D3D11_MAP_WRITE_DISCARD, 0, &directionalLight);
	memcpy(directionalLight.pData, &direction, sizeof(DIRECTIONAL_LIGHT));
	deviceContext->Unmap(constantBufferDirectional, 0);

	D3D11_MAPPED_SUBRESOURCE map_camera;
	deviceContext->Map(constantBufferScene, 0, D3D11_MAP_WRITE_DISCARD, 0, &map_camera);
	memcpy(map_camera.pData, &camera, sizeof(SCENE));
	deviceContext->Unmap(constantBufferScene, 0);

	deviceContext->RSSetViewports(1, &viewport);

#pragma region Skybox

	deviceContext->RSSetState(rasterstate);

	D3D11_MAPPED_SUBRESOURCE skybox_map;
	deviceContext->Map(constantBufferObj, 0, D3D11_MAP_WRITE_DISCARD, 0, &skybox_map);
	memcpy(skybox_map.pData, &skybox_1, sizeof(OBJECT));
	deviceContext->Unmap(constantBufferObj, 0);

	UINT strideSize_sky = sizeof(CUBE);
	UINT strideOffset_sky = 0;
	deviceContext->IASetVertexBuffers(0, 1, &vertexBufferSkybox.p, &strideSize_sky, &strideOffset_sky);
	deviceContext->IASetIndexBuffer(indexBufferSkybox, DXGI_FORMAT_R32_UINT, 0);

	deviceContext->VSSetShader(vShader2, nullptr, 0);
	deviceContext->PSSetShader(pShader2, nullptr, 0);
	deviceContext->GSSetShader(NULL, NULL, NULL);

	deviceContext->PSSetShaderResources(0, 1, &srv1.p);

	deviceContext->IASetInputLayout(input2);

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	deviceContext->DrawIndexed(36, 0, 0);

	deviceContext->ClearDepthStencilView(dsView, D3D11_CLEAR_DEPTH, 1, 0);

	deviceContext->RSSetState(rasterstate1);

#pragma endregion

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
	
	/*deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	deviceContext->DrawIndexed(import.uniqueIndicies.size(), 0, 0);*/
	
#pragma endregion

#pragma region Cubes

	D3D11_MAPPED_SUBRESOURCE cloneMap;

	deviceContext->Map(constantBufferInstance, 0, D3D11_MAP_WRITE_DISCARD, 0, &cloneMap);
	memcpy(cloneMap.pData, clones, sizeof(clones));
	deviceContext->Unmap(constantBufferInstance, 0);

	UINT strideSize2 = sizeof(CUBE);
	UINT strideOffset2 = 0;
	deviceContext->IASetVertexBuffers(0, 1, &vertexBufferCube.p, &strideSize2, &strideOffset2);
	deviceContext->IASetIndexBuffer(indexBufferCube, DXGI_FORMAT_R32_UINT, 0);

	deviceContext->VSSetShader(vShaderInstance, nullptr, 0);
	deviceContext->PSSetShader(pShader, nullptr, 0);

	deviceContext->IASetInputLayout(input);

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	deviceContext->DrawIndexedInstanced(36, 36, 0, 0, 0);

#pragma endregion

#pragma region Line

	D3D11_MAPPED_SUBRESOURCE map1;
	deviceContext->Map(constantBufferInstanceLine, 0, D3D11_MAP_WRITE_DISCARD, 0, &map1);
	memcpy(map1.pData, clones, sizeof(clones));
	deviceContext->Unmap(constantBufferInstanceLine, 0);

	UINT strideSize1 = sizeof(LINE);
	UINT strideOffset1 = 0;
	deviceContext->IASetVertexBuffers(0, 1, &vertexBufferLine.p, &strideSize1, &strideOffset1);
	deviceContext->IASetIndexBuffer(indexBufferLine, DXGI_FORMAT_R32_UINT, 0);

	deviceContext->VSSetShader(vShaderInstanceLines, nullptr, 0);
	deviceContext->PSSetShader(pShader1, nullptr, 0);
	deviceContext->IASetInputLayout(input1);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	deviceContext->DrawIndexed(62, 0, 0);

#pragma endregion
	
	swapchain->Present(0, 0);

	frame++;

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
		temp.m128_f32[2] = 0.05f;
		temp.m128_f32[3] = camera.viewMatrix.r[3].m128_f32[3];

		camera.viewMatrix = XMMatrixMultiply(DirectX::XMMatrixTranslationFromVector(temp), camera.viewMatrix);
	}

	if (GetAsyncKeyState('S'))
	{
		DirectX::XMVECTOR temp;
		temp.m128_f32[0] = 0;
		temp.m128_f32[1] = 0;
		temp.m128_f32[2] = -0.05f;
		temp.m128_f32[3] = camera.viewMatrix.r[3].m128_f32[3];

		camera.viewMatrix = XMMatrixMultiply(DirectX::XMMatrixTranslationFromVector(temp), camera.viewMatrix);
	}

	if (GetAsyncKeyState('A'))
	{
		DirectX::XMVECTOR temp;
		temp.m128_f32[0] = -0.05f;
		temp.m128_f32[1] = 0;
		temp.m128_f32[2] = 0;
		temp.m128_f32[3] = camera.viewMatrix.r[3].m128_f32[3];

		camera.viewMatrix = XMMatrixMultiply(DirectX::XMMatrixTranslationFromVector(temp), camera.viewMatrix);
	}

	if (GetAsyncKeyState('D'))
	{
		DirectX::XMVECTOR temp;
		temp.m128_f32[0] = 0.05f;
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

	skybox_1.worldMatrix.r[3].m128_f32[0] = camera.viewMatrix.r[3].m128_f32[0];
	skybox_1.worldMatrix.r[3].m128_f32[1] = camera.viewMatrix.r[3].m128_f32[1];
	skybox_1.worldMatrix.r[3].m128_f32[2] = camera.viewMatrix.r[3].m128_f32[2];
	skybox_1.worldMatrix.r[3].m128_f32[3] = camera.viewMatrix.r[3].m128_f32[3];

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

DirectX::XMMATRIX RTA_PROJECT::ConvertFBX_DX11(FbxAMatrix _m)
{
	DirectX::XMMATRIX temp;
	
	temp.r[0].m128_f32[0] = (float)_m[0].mData[0];
	temp.r[0].m128_f32[1] = (float)_m[0].mData[1];
	temp.r[0].m128_f32[2] = (float)_m[0].mData[2];
	temp.r[0].m128_f32[3] = (float)_m[0].mData[3];

	temp.r[1].m128_f32[0] = (float)_m[1].mData[0];
	temp.r[1].m128_f32[1] = (float)_m[1].mData[1];
	temp.r[1].m128_f32[2] = (float)_m[1].mData[2];
	temp.r[1].m128_f32[3] = (float)_m[1].mData[3];

	temp.r[2].m128_f32[0] = (float)_m[2].mData[0];
	temp.r[2].m128_f32[1] = (float)_m[2].mData[1];
	temp.r[2].m128_f32[2] = (float)_m[2].mData[2];
	temp.r[2].m128_f32[3] = (float)_m[2].mData[3];

	temp.r[3].m128_f32[0] = (float)_m[3].mData[0];
	temp.r[3].m128_f32[1] = (float)_m[3].mData[1];
	temp.r[3].m128_f32[2] = (float)_m[3].mData[2];
	temp.r[3].m128_f32[3] = (float)_m[3].mData[3];

	return temp;

}

