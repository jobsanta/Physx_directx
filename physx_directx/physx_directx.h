#pragma once

#include "resource.h"
#include <iostream>
#include <windowsx.h>
#include "GameTimer.h"
//-- DirecX Library
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXColors.h>

//-- Utilites Library
#include "Camera.h"

//-- Physx Library
#include <PxPhysicsAPI.h>
#include <PxExtensionsAPI.h>
#include <PxDefaultErrorCallback.h>
#include <PxDefaultAllocator.h>
#include <PxDefaultSimulationFilterShader.h>
#include <PxDefaultCpuDispatcher.h>
#include <PxShapeExt.h>
#include <PxMat33.h>
#include <PxSimpleFactory.h>
#include <vector>
#include <PxVisualDebuggerExt.h>


//---DirectxTK Library
#include <Effects.h>
#include <PrimitiveBatch.h>
#include <GeometricPrimitive.h>
#include <VertexTypes.h>

#include <algorithm>

#define MAX_LOADSTRING 100


using namespace std;
using namespace DirectX;
using namespace physx;

class PhysxApp
{
public:
    PhysxApp(HINSTANCE hInstance);
    ~PhysxApp();

    HINSTANCE           AppInst()const;
    HWND                MainWnd()const;
    float               AspectRatio() const;
	LRESULT				MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	bool Init();
	bool InitMainWindow();
	bool InitDirect3D();
	int Run();

	static PxPhysics*  gPhysicsSDK;
	static PxDefaultErrorCallback    gDefaultErrorCallback;
	static PxDefaultAllocator        gDefaultAllocatorCallback;
	static PxSimulationFilterShader  gDefaultFilterShader;
	static PxFoundation*             gFoundation;
protected:
	HINSTANCE           hInstance;								         //current instance
    HWND                g_hWnd = nullptr;
    TCHAR               szTitle[MAX_LOADSTRING];					 //The title bar text
    TCHAR               szWindowClass[MAX_LOADSTRING];			     //the main window class name
	int nCmdShow;



    //--------------------------------------------------------------------------------------
    //Physx       Global                                        Variables
    //--------------------------------------------------------------------------------------


    PxScene*      gScene                                                       = NULL;
    PxReal        myTimestep                                                   = 1.0f / 6000.0f;
    PxRigidActor* box;



	HINSTANCE     mhAppInst;
	HWND          mhMainWnd;
	bool          mAppPaused;
	bool          mMinimized;
	bool          mMaximized;
	bool          mResizing;
	UINT          m4xMsaaQuality;
    GameTimer     mTimer;

	//              Derived class should set these in derived constructor to customize starting values.
	std::wstring    mMainWndCaption;
	D3D_DRIVER_TYPE md3dDriverType;
	int             mClientWidth;
	int             mClientHeight;
	bool            mEnable4xMsaa;

    //--------------------------------------------------------------------------------------
    //DirectX Global
    //--------------------------------------------------------------------------------------
	D3D_DRIVER_TYPE         g_driverType;
	D3D_FEATURE_LEVEL       g_featureLevel;
	ID3D11Device*           g_pd3dDevice;
    ID3D11Device1*          g_pd3dDevice1;
    ID3D11DeviceContext*    g_pImmediateContext;
    ID3D11DeviceContext1*   g_pImmediateContext1;
    IDXGISwapChain*         g_pSwapChain;
    IDXGISwapChain1*        g_pSwapChain1;

    ID3D11RenderTargetView* g_pRenderTargetView;
    ID3D11InputLayout*		g_pBatchInputLayout;

    std::unique_ptr<BasicEffect>                         g_BatchEffect;
    std::unique_ptr<GeometricPrimitive>                  g_Shape;
    std::unique_ptr<PrimitiveBatch<VertexPositionColor>> g_Batch;



    XMMATRIX                g_World;
    XMMATRIX                g_View;
    XMMATRIX                g_Projection;
    Camera                  mCam;
    POINT                   mLastMousePos;


	ATOM				    MyRegisterClass(HINSTANCE hInstance);
	BOOL				    InitInstance(HINSTANCE, int);


	//DirectX               Function
    HRESULT                 InitDevice();
	void                    CleanupDevice();
    void                    Render();
	void                    UpdateScene(float dt);

	//Draw                  Function
	void                    DrawBox(PxShape* pShape, PxRigidActor* actor);
	void                    DrawShape(PxShape* shape, PxRigidActor* actor);
	void                    DrawActor(PxRigidActor* actor);
	void                    DrawGrid(PrimitiveBatch<VertexPositionColor>& batch, FXMVECTOR xAxis, FXMVECTOR yAxis,
		                    FXMVECTOR    origin, size_t xdivs, size_t ydivs, GXMVECTOR color);
	void                    RenderActors();

	//PHYSX                 Function
	void                    InitializePhysX();
	void                    ShutdownPhysX();
	void                    StepPhysX();

	//Mouse                 Function
	void                    OnMouseDown(WPARAM btnState, int x, int y);
	void                    OnMouseUp(WPARAM btnState, int x, int y);
	void                    OnMouseMove(WPARAM btnState, int x, int y);


	//Helper                Function 
	XMMATRIX                PxtoXMMatrix(PxTransform input);




};
