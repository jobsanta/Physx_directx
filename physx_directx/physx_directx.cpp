// physx_directx.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "physx_directx.h"
#include <windowsx.h>

PxPhysics*                PhysxApp::gPhysicsSDK = NULL;
PxDefaultErrorCallback    PhysxApp::gDefaultErrorCallback;
PxDefaultAllocator        PhysxApp::gDefaultAllocatorCallback;
PxSimulationFilterShader  PhysxApp::gDefaultFilterShader = PxDefaultSimulationFilterShader;
PxFoundation*             PhysxApp::gFoundation = NULL;


namespace
{
	PhysxApp* app = 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	return app->MsgProc(hWnd, message, wParam, lParam);

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

PhysxApp::PhysxApp(HINSTANCE hInstance)
	:
	hInstance(hInstance),
	g_hWnd(0),
	nWidth(800),
	nHeight(600),
	g_pd3dDevice(0),
	g_pd3dDevice1(0),
	g_pImmediateContext(0),
	g_pSwapChain(0),
	g_pSwapChain1(0),
	g_pRenderTargetView(0),
	g_pBatchInputLayout(0),
	theConnection(0)
{
	app = this;
}

PhysxApp::~PhysxApp()
{
	CleanupDevice();
	ShutdownPhysX();
}

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	PhysxApp theApp(hInstance);
	theApp.nCmdShow = nCmdShow;
	if (!theApp.Init())
		return 0;

	return theApp.Run();
}

bool PhysxApp::Init()
{
	if (!InitMainWindow())
		return false;

	if (FAILED(InitDevice()))
		return false;

	InitializePhysX();
	

	return true;
}

bool PhysxApp::InitMainWindow()
{
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PHYSX_DIRECTX));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDC_PHYSX_DIRECTX);
	wcex.lpszClassName = L"Physx_DirectX_Class";
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	
	if (!RegisterClassEx(&wcex))
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}

	g_hWnd = CreateWindow(L"Physx_DirectX_Class", L"Physx_DirectX", WS_OVERLAPPEDWINDOW,nWidth, 0,nHeight, 0, NULL, NULL, hInstance, NULL);

	if (!g_hWnd)
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	ShowWindow(g_hWnd, nCmdShow);
	UpdateWindow(g_hWnd);
	return true;

}

int PhysxApp::Run()
{
	mTimer.Reset();
	MSG msg;
	// Main message loop:
	msg = { 0 };
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			mTimer.Tick();
			UpdateScene(mTimer.DeltaTime());
			Render();
		}
	}
	
	return (int)msg.wParam;

}

LRESULT PhysxApp::MsgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;
	case WM_MOUSEMOVE:
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void PhysxApp::UpdateScene(float dt)
{
	//
	// Control the camera.
	//
	if (GetAsyncKeyState('W') & 0x8000)
		mCam.Walk(10.0f*dt);

	if (GetAsyncKeyState('S') & 0x8000)
		mCam.Walk(-10.0f*dt);

	if (GetAsyncKeyState('A') & 0x8000)
		mCam.Strafe(-10.0f*dt);

	if (GetAsyncKeyState('D') & 0x8000)
		mCam.Strafe(10.0f*dt);

}

void PhysxApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(g_hWnd);
}

void PhysxApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void PhysxApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

		mCam.Pitch(dy);
		mCam.RotateY(dx);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

bool PhysxApp::PickActor(int x, int y)
{
	LetGoActor();

	Ray ray;
	XMVECTOR vector1 = XMVector3Unproject(
		XMVectorSet(x, y, 0.0f, 1.0f),
		0.0f,
		0.0f,
		nWidth,
		nHeight,
		0.0f,
		1.0f,
		g_Projection,
		g_View,
		g_World);

	XMVECTOR vector2 = XMVector3Unproject(
		XMVectorSet(x, y, 1.0f, 1.0f),
		0.0f,
		0.0f,
		nWidth,
		nHeight,
		0.0f,
		1.0f,
		g_Projection,
		g_View,
		g_World);

	XMVECTOR normalize = XMVector4Normalize(vector2);
	ray.orig = PxVec3(XMVectorGetX(vector1), XMVectorGetY(vector1), XMVectorGetZ(vector1));
	ray.dir = PxVec3(XMVectorGetX(normalize), XMVectorGetY(normalize), XMVectorGetZ(normalize));
	ray.distance = XMVectorGetX( XMVector4Length(vector2) );

	PxRaycastBuffer hit;
	bool status = gScene->raycast(ray.orig, ray.dir, ray.distance, hit);

	if (!hit.hasBlock) return false;
	PxRigidActor* actor = hit.block.actor;
	if (!actor->isRigidDynamic()) return false;

	//int hitx, hity;
	//XMVECTOR hitVector;
	//XMStoreFloat3(&XMFLOAT3(hit.block.position.x, hit.block.position.y, hit.block.position.z),hitVector);
	//XMVector3Project(hitVector,
	//	0.0f,
	//	0.0f,
	//	nWidth,
	//	nHeight,
	//	0.0f,
	//	1.0f,
	//	g_Projection,
	//	g_View,
	//	g_World);
	//ViewProject(hit.block.position, hitx, hity, gMouseDepth);
	gMouseSphere = CreateSphere(hit.block.position, 0.1f, 1.0f);
	gMouseSphere->setRigidDynamicFlag(PxRigidDynamicFlag::eKINEMATIC, true);

	gSelectedActor = (PxRigidDynamic*)hit.block.actor;
	gSelectedActor->wakeUp();

	PxTransform mFrame, sFrame;
	mFrame.q = gMouseSphere->getGlobalPose().q;
	mFrame.p = gMouseSphere->getGlobalPose().transformInv(hit.block.position);
	sFrame.q = gSelectedActor->getGlobalPose().q;
	sFrame.p = gSelectedActor->getGlobalPose().transformInv(hit.block.position);

	gMouseJoint = PxDistanceJointCreate(*gPhysicsSDK, gMouseSphere, mFrame, gSelectedActor, sFrame);

	gMouseJoint->setDamping(1);
	gMouseJoint->setStiffness(200);
	gMouseJoint->setMinDistance(0);
	gMouseJoint->setMaxDistance(0);
	gMouseJoint->setDistanceJointFlag(PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, true);
	gMouseJoint->setDistanceJointFlag(PxDistanceJointFlag::eSPRING_ENABLED, true);

	return true;
}

void PhysxApp::LetGoActor()
{
	if (gMouseJoint)
		gMouseJoint->release();
	gMouseJoint = NULL;

	if (gMouseSphere)
		gMouseSphere->release();
	gMouseSphere = NULL;
}


PxRigidDynamic* PhysxApp::CreateSphere(const PxVec3& pos, const PxReal radius, const PxReal density)
{
	PxTransform transform(pos, PxQuat::createIdentity());
	PxSphereGeometry geometry(radius);

	PxMaterial* mMaterial = gPhysicsSDK->createMaterial(0.5, 0.5, 0.5);

	PxRigidDynamic* actor = PxCreateDynamic(*gPhysicsSDK, transform, geometry, *mMaterial, density);
	if (!actor)
		cerr << "create actor failed" << endl;
	actor->setAngularDamping(0.75);
	actor->setLinearVelocity(PxVec3(0, 0, 0));
	gScene->addActor(*actor);
	return actor;
}

void PhysxApp::StepPhysX()
{
	gScene->simulate(myTimestep);

	while (!gScene->fetchResults())
	{

	}
}

XMMATRIX PhysxApp::PxtoXMMatrix(PxTransform input)
{
	PxMat33 quat = PxMat33(input.q);
	XMFLOAT4X4 start;
	XMMATRIX mat;
	start._11 = quat.column0[0];
	start._12 = quat.column0[1];
	start._13 = quat.column0[2];
	start._14 = 0;


	start._21 = quat.column1[0];
	start._22 = quat.column1[1];
	start._23 = quat.column1[2];
	start._24 = 0;

	start._31 = quat.column2[0];
	start._32 = quat.column2[1];
	start._33 = quat.column2[2];
	start._34 = 0;

	start._41 = input.p.x;
	start._42 = input.p.y;
	start._43 = input.p.z;
	start._44 = 1;

	mat = XMLoadFloat4x4(&start);
	return mat;
}

void PhysxApp::DrawBox(PxShape* pShape, PxRigidActor* actor)
{
	PxTransform pT = PxShapeExt::getGlobalPose(*pShape,*actor);
	PxBoxGeometry bg;
	pShape->getBoxGeometry(bg);
	XMMATRIX mat = PxtoXMMatrix(pT);
	g_Shape->Draw(mat, g_View, g_Projection);
}

void PhysxApp::DrawShape(PxShape* shape, PxRigidActor* actor)
{
	PxGeometryType::Enum type = shape->getGeometryType();
	switch (type)
	{
	case PxGeometryType::eBOX:
		DrawBox(shape,actor);
		break;
	case PxGeometryType::eSPHERE:
		
	}
}

void PhysxApp::DrawActor(PxRigidActor* actor)
{
	PxU32 nShapes = actor->getNbShapes();
	PxShape** shapes = new PxShape*[nShapes];

	actor->getShapes(shapes, nShapes);
	while (nShapes--)
	{
		DrawShape(shapes[nShapes], actor);
	}
	delete[] shapes;
}

void PhysxApp::RenderActors()
{

	for (int i = 0; i < boxes.size(); i++)
		DrawActor(boxes[i]);
}

void PhysxApp::InitializePhysX() {

	gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gDefaultAllocatorCallback, gDefaultErrorCallback);

	gPhysicsSDK = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale());
	if (gPhysicsSDK == NULL) {
		cerr << "Error creating PhysX3 device." << endl;
		cerr << "Exiting..." << endl;
		exit(1);
	}

	if (!PxInitExtensions(*gPhysicsSDK))
		cerr << "PxInitExtensions failed!" << endl;

	//if (gPhysicsSDK->getPvdConnectionManager() == NULL)
	//	return;
	//const char* pvd_host_ip = "127.0.0.1";
	//int port = 5425;
	//unsigned int timeout = 100;
	//
	//
	////--- Debugger
	//PxVisualDebuggerConnectionFlags connectionFlags = PxVisualDebuggerExt::getAllConnectionFlags();
	//theConnection = PxVisualDebuggerExt::createConnection(gPhysicsSDK->getPvdConnectionManager(),
	//	pvd_host_ip, port, timeout, connectionFlags);


	//Create the scene
	PxSceneDesc sceneDesc(gPhysicsSDK->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.8f, 0.0f);

	if (!sceneDesc.cpuDispatcher) {
		PxDefaultCpuDispatcher* mCpuDispatcher = PxDefaultCpuDispatcherCreate(1);
		if (!mCpuDispatcher)
			cerr << "PxDefaultCpuDispatcherCreate failed!" << endl;
		sceneDesc.cpuDispatcher = mCpuDispatcher;
	}
	if (!sceneDesc.filterShader)
		sceneDesc.filterShader  = gDefaultFilterShader;


	gScene = gPhysicsSDK->createScene(sceneDesc);
	if (!gScene)
		cerr << "createScene failed!" << endl;

	gScene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0);
	gScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);


	PxMaterial* mMaterial = gPhysicsSDK->createMaterial(0.5, 0.5, 0.5);

	//Create actors 
	//1) Create ground plane
	PxReal d              = 0.0f;
	PxTransform pose      = PxTransform(PxVec3(0.0f, 0, 0.0f), PxQuat(PxHalfPi, PxVec3(0.0f, 0.0f, 1.0f)));

	PxRigidStatic* plane  = gPhysicsSDK->createRigidStatic(pose);
	if (!plane)
		cerr << "create plane failed!" << endl;

	PxShape* shape        = plane->createShape(PxPlaneGeometry(), *mMaterial);
	if (!shape)
		cerr << "create shape failed!" << endl;
	gScene->addActor(*plane);


	//2)           Create cube	 
	PxReal         density = 1.0f;
	PxTransform    transform(PxVec3(0.0f, 10.0f, 0.0f), PxQuat::createIdentity());
	PxVec3         dimensions(0.5f,0.5f,0.5f);
	PxBoxGeometry  geometry(dimensions);

	for (int i = 0; i < 10; i++)
	{
		transform.p = PxVec3(0.0f, 5.0f + 5 * i, 0.0f);
		PxRigidDynamic *actor = PxCreateDynamic(*gPhysicsSDK, transform, geometry, *mMaterial, density);

		actor->setAngularDamping(0.75);
		actor->setLinearVelocity(PxVec3(0, 0, 0));
		if (!actor)
			cerr << "create actor failed!" << endl;
		gScene->addActor(*actor);
		boxes.push_back(actor);
	}


}

void PhysxApp::ShutdownPhysX() {

	if (gScene != NULL)
	{
		for (int i = 0; i < boxes.size(); i++)
			gScene->removeActor(*boxes[i]);

		gScene->release();

		for (int i = 0; i < boxes.size(); i++)
			boxes[i]->release();
	}
	if (gPhysicsSDK!=NULL)
		gPhysicsSDK->release();
	
	//if (theConnection!=NULL)
	//	theConnection->release();
}

HRESULT PhysxApp::InitDevice()
{
	HRESULT hr = S_OK;

	RECT rc;
	GetClientRect(g_hWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

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
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		g_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDevice(nullptr, g_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);

		if (hr == E_INVALIDARG)
		{
			// DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
			hr = D3D11CreateDevice(nullptr, g_driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
				D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);
		}

		if (SUCCEEDED(hr))
			break;
	}
	if (FAILED(hr))
		return hr;

	// Obtain DXGI factory from device (since we used nullptr for pAdapter above)
	IDXGIFactory1* dxgiFactory = nullptr;
	{
		IDXGIDevice* dxgiDevice = nullptr;
		hr = g_pd3dDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
		if (SUCCEEDED(hr))
		{
			IDXGIAdapter* adapter = nullptr;
			hr = dxgiDevice->GetAdapter(&adapter);
			if (SUCCEEDED(hr))
			{
				hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));
				adapter->Release();
			}
			dxgiDevice->Release();
		}
	}
	if (FAILED(hr))
		return hr;

	// Create swap chain
	IDXGIFactory2* dxgiFactory2 = nullptr;
	hr = dxgiFactory->QueryInterface(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory2));
	if (dxgiFactory2)
	{
		// DirectX 11.1 or later
		hr = g_pd3dDevice->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(&g_pd3dDevice1));
		if (SUCCEEDED(hr))
		{
			(void)g_pImmediateContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&g_pImmediateContext1));
		}

		DXGI_SWAP_CHAIN_DESC1 sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.Width = width;
		sd.Height = height;
		sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = 1;

		hr = dxgiFactory2->CreateSwapChainForHwnd(g_pd3dDevice, g_hWnd, &sd, nullptr, nullptr, &g_pSwapChain1);
		if (SUCCEEDED(hr))
		{
			hr = g_pSwapChain1->QueryInterface(__uuidof(IDXGISwapChain), reinterpret_cast<void**>(&g_pSwapChain));
		}

		dxgiFactory2->Release();
	}
	else
	{
		// DirectX 11.0 systems
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

		hr = dxgiFactory->CreateSwapChain(g_pd3dDevice, &sd, &g_pSwapChain);
	}

	// Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
	dxgiFactory->MakeWindowAssociation(g_hWnd, DXGI_MWA_NO_ALT_ENTER);

	dxgiFactory->Release();

	if (FAILED(hr))
		return hr;

	// Create a render target view
	ID3D11Texture2D* pBackBuffer = nullptr;
	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
	if (FAILED(hr))
		return hr;

	hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_pRenderTargetView);
	pBackBuffer->Release();
	if (FAILED(hr))
		return hr;



	// Create depth stencil texture
	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = width;
	descDepth.Height = height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	hr = g_pd3dDevice->CreateTexture2D(&descDepth, nullptr, &g_pDepthStencil);
	if (FAILED(hr))
		return hr;

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	hr = g_pd3dDevice->CreateDepthStencilView(g_pDepthStencil, &descDSV, &g_pDepthStencilView);
	if (FAILED(hr))
		return hr;

	std::unique_ptr<CommonStates> states(new CommonStates(g_pd3dDevice));
	g_pImmediateContext->OMSetDepthStencilState(states->DepthDefault(), 0);
	g_pImmediateContext->RSSetState(states->CullCounterClockwise());
	g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);

	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_pImmediateContext->RSSetViewports(1, &vp);

	g_Batch.reset(new PrimitiveBatch<VertexPositionColor>(g_pImmediateContext));
	
	g_BatchEffect.reset(new BasicEffect(g_pd3dDevice));
	g_BatchEffect->SetVertexColorEnabled(true);

	void const* shaderByteCode;
	size_t byteCodeLength;

	g_BatchEffect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

	hr = g_pd3dDevice->CreateInputLayout(VertexPositionColor::InputElements,
		VertexPositionColor::InputElementCount,
		shaderByteCode, byteCodeLength,
		&g_pBatchInputLayout);
	if (FAILED(hr))
		return hr;

	// Initialize the world matrix
	g_World = XMMatrixIdentity();

	// Initialize the view matrix
	mCam.SetPosition(0.0f, 3.0f, -8.0f);
	g_View = mCam.View();

	g_BatchEffect->SetView(g_View);


	g_Shape = GeometricPrimitive::CreateCube(g_pImmediateContext, 1.0f,false);

	// Initialize the projection matrix
	//g_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, width / (FLOAT)height, 0.01f, 100.0f);
	mCam.SetLens(XM_PIDIV2, width / (FLOAT)height, 0.01f, 100.0f);


	g_BatchEffect->SetProjection(mCam.Proj());

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Render a grid using PrimitiveBatch
//--------------------------------------------------------------------------------------
void PhysxApp::DrawGrid(PrimitiveBatch<VertexPositionColor>& batch, FXMVECTOR xAxis, FXMVECTOR yAxis, FXMVECTOR origin, size_t xdivs, size_t ydivs, GXMVECTOR color)
{
	g_BatchEffect->Apply(g_pImmediateContext);

	g_pImmediateContext->IASetInputLayout(g_pBatchInputLayout);

	g_Batch->Begin();

	xdivs = std::max<size_t>(1, xdivs);
	ydivs = std::max<size_t>(1, ydivs);

	for (size_t i = 0; i <= xdivs; ++i)
	{
		float fPercent = float(i) / float(xdivs);
		fPercent = (fPercent * 2.0f) - 1.0f;
		XMVECTOR vScale = XMVectorScale(xAxis, fPercent);
		vScale = XMVectorAdd(vScale, origin);

		VertexPositionColor v1(XMVectorSubtract(vScale, yAxis), color);
		VertexPositionColor v2(XMVectorAdd(vScale, yAxis), color);
		batch.DrawLine(v1, v2);
	}

	for (size_t i = 0; i <= ydivs; i++)
	{
		FLOAT fPercent = float(i) / float(ydivs);
		fPercent = (fPercent * 2.0f) - 1.0f;
		XMVECTOR vScale = XMVectorScale(yAxis, fPercent);
		vScale = XMVectorAdd(vScale, origin);

		VertexPositionColor v1(XMVectorSubtract(vScale, xAxis), color);
		VertexPositionColor v2(XMVectorAdd(vScale, xAxis), color);
		batch.DrawLine(v1, v2);
	}

	g_Batch->End();
}

void PhysxApp::CleanupDevice()
{
	if (g_pImmediateContext)  g_pImmediateContext->ClearState();
	if (g_pBatchInputLayout)  g_pBatchInputLayout->Release();
	if (g_pDepthStencilView)  g_pDepthStencilView->Release();
	if (g_pDepthStencil)	  g_pDepthStencil->Release();
	if (g_pRenderTargetView)  g_pRenderTargetView->Release();
	if (g_pSwapChain1)        g_pSwapChain1->Release();
	if (g_pSwapChain)         g_pSwapChain->Release();
	if (g_pImmediateContext1) g_pImmediateContext1->Release();
	if (g_pImmediateContext)  g_pImmediateContext->Release();
	if (g_pd3dDevice1)        g_pd3dDevice1->Release();
	if (g_pd3dDevice)         g_pd3dDevice->Release();
	

}

void PhysxApp::Render()
{
	// Update our time
	static float t = 0.0f;
	static float dt = 0.f;
	if (g_driverType == D3D_DRIVER_TYPE_REFERENCE)
	{
		t += (float)XM_PI * 0.0125f;
	}
	else
	{
		static uint64_t dwTimeStart = 0;
		static uint64_t dwTimeLast = 0;
		uint64_t dwTimeCur = GetTickCount64();
		if (dwTimeStart == 0)
			dwTimeStart = dwTimeCur;
		t = (dwTimeCur - dwTimeStart) / 1000.0f;
		dt = (dwTimeCur - dwTimeLast) / 1000.0f;
		dwTimeLast = dwTimeCur;
	}

	mCam.UpdateViewMatrix();
	g_View = mCam.View();
	g_Projection = mCam.Proj();


	g_BatchEffect->SetView(g_View);
	g_BatchEffect->SetProjection(g_Projection);


	if (gScene)
	{
		StepPhysX();
	}

	g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, Colors::MidnightBlue);

	g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	const XMVECTORF32 xaxis = { 20.0f, 0.f, 0.f };
	const XMVECTORF32 yaxis = { 0.f, 0.f, 20.f };
	DrawGrid(*g_Batch, xaxis, yaxis, g_XMZero, 20, 20, Colors::Gray);

	RenderActors();



	g_pSwapChain->Present(0, 0 );

}




