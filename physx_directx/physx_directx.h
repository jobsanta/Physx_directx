#pragma once

#include "resource.h"
#include "GameTimer.h"
#include <iostream>
#include <windowsx.h>
#include <algorithm>

//-- DirecX Library
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXColors.h>

//---DirectxTK Library
#include <Effects.h>
#include <PrimitiveBatch.h>
#include <GeometricPrimitive.h>
#include <VertexTypes.h>
#include <CommonStates.h>

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


#define MAX_LOADSTRING 100


using namespace std;
using namespace DirectX;
using namespace physx;

class PhysxApp
{
public:
    PhysxApp(HINSTANCE hInstance);
    ~PhysxApp();

	int                 nCmdShow;
	int                 nWidth;
	int                 nHeight;
    float               AspectRatio() const;
	HWND                g_hWnd = nullptr;
	HINSTANCE           hInstance;								         //current instance
	LRESULT				MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	
	static PxPhysics*                gPhysicsSDK;
	static PxDefaultErrorCallback    gDefaultErrorCallback;
	static PxDefaultAllocator        gDefaultAllocatorCallback;
	static PxSimulationFilterShader  gDefaultFilterShader;
	static PxFoundation*             gFoundation;

	int  Run();
	bool Init();
	bool InitMainWindow();
	bool InitDirect3D();



protected:
	//--------------------------------------------------------------------------------------
    //Physx       
    //--------------------------------------------------------------------------------------
	PxScene*              gScene         = NULL;
    PxReal                myTimestep     = 1.0f / 600.0f;
    vector<PxRigidActor*> boxes;
	PxDistanceJoint*      gMouseJoint    = NULL;
	PxRigidDynamic*       gMouseSphere   = NULL;
	PxReal                gMouseDepth    = 0.0f;
	PxRigidDynamic*       gSelectedActor = NULL;

	struct Ray{
		PxVec3 orig, dir;
		PxReal distance;
	};


	PxVisualDebuggerConnection* theConnection;

	GameTimer                           mTimer;
    //--------------------------------------------------------------------------------------
    //DirectX 
    //--------------------------------------------------------------------------------------
	D3D_DRIVER_TYPE                     g_driverType;
	D3D_FEATURE_LEVEL                   g_featureLevel;
	ID3D11Device*                       g_pd3dDevice;
    ID3D11Device1*                      g_pd3dDevice1;
    ID3D11DeviceContext*                g_pImmediateContext;
    ID3D11DeviceContext1*               g_pImmediateContext1;
    IDXGISwapChain*                     g_pSwapChain;
    IDXGISwapChain1*                    g_pSwapChain1;
	ID3D11Texture2D*                    g_pDepthStencil = nullptr;
	ID3D11DepthStencilView*             g_pDepthStencilView = nullptr;

    ID3D11RenderTargetView*             g_pRenderTargetView;
    ID3D11InputLayout*		            g_pBatchInputLayout;



	XMMATRIX                            g_World;
	XMMATRIX                            g_View;
	XMMATRIX                            g_Projection;
	Camera                              mCam;
	POINT                               mLastMousePos;

    std::unique_ptr<BasicEffect>                         g_BatchEffect;
    std::unique_ptr<GeometricPrimitive>                  g_Shape;
    std::unique_ptr<PrimitiveBatch<VertexPositionColor>> g_Batch;
	
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
	void	                DrawSphere(PxShape* pShape, PxRigidActor* actor);
	void                    RenderActors();

	//PHYSX                 Function
	void                    InitializePhysX();
	void                    ShutdownPhysX();
	void                    StepPhysX();
	PxRigidDynamic*         CreateSphere(const PxVec3& pos, const PxReal radius, const PxReal density);

	//Mouse                 Function
	void                    OnMouseDown(WPARAM btnState, int x, int y);
	void                    OnMouseUp(WPARAM btnState, int x, int y);
	void                    OnMouseMove(WPARAM btnState, int x, int y);
	bool                    PickActor(int x, int y);
	void                    MoveActor(int x, int y);
	void                    LetGoActor();


	//Helper                Function 
	XMMATRIX                PxtoXMMatrix(PxTransform input);




};
