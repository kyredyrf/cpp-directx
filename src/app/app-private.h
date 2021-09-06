#pragma once

#include "directx/d3d12.h"
#include "directx/d3dx12.h"
#include <array>
#include <d3dcompiler.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <DirectXMath.h>

using Microsoft::WRL::ComPtr;
using DirectX::XMFLOAT3;
using DirectX::XMFLOAT2;

inline void ThrowIfFailed(HRESULT hr, const std::initializer_list<HRESULT>& excludeList = { }, const std::source_location& location = std::source_location::current())
{
    if (std::find_if(excludeList.begin(), excludeList.end(), [&](auto x) { return x == hr; }) != excludeList.end())
    {
        return;
    }

    if (FAILED(hr))
    {
        Exception(location, "hr=%xH", hr);
    }
}

static std::vector<UINT8> GenerateTextureData(int width, int height, int pixelSize);

/**
 *  アプリ非公開データ
 */
struct App::Private
{
    Private(App* app);
    ~Private();

    void LoadPipeline();
    void LoadFactory();
    void LoadAdapter();
    void LoadDevice();
    void LoadAssets();
    void PopulateCommandList();
    void WaitForPreviousFrame();

    static const UINT FrameCount = 2;
    static const UINT TextureWidth = 256;
    static const UINT TextureHeight = 256;
    static const UINT TexturePixelSize = 4;    // The number of bytes used to represent a pixel in the texture.

    App* app;

    ComPtr<IDXGIFactory> factory;
    ComPtr<IDXGIFactory1> factory1;
    ComPtr<IDXGIFactory2> factory2;
    ComPtr<IDXGIFactory3> factory3;
    ComPtr<IDXGIFactory4> factory4;
    ComPtr<IDXGIFactory5> factory5;
    ComPtr<IDXGIFactory6> factory6;
    ComPtr<IDXGIFactory7> factory7;

    DXGI_GPU_PREFERENCE gpuReference;
    ComPtr<IDXGIAdapter> adapter;
    ComPtr<IDXGIAdapter1> adapter1;
    ComPtr<IDXGIAdapter2> adapter2;
    ComPtr<IDXGIAdapter3> adapter3;
    ComPtr<IDXGIAdapter4> adapter4;

    D3D_FEATURE_LEVEL MinimumFeatureLevel;
    // ComPtr<ID3D12Device> m_device;
    ID3D12Device* m_device;

    ComPtr<ID3D12CommandQueue> m_commandQueue;
    ComPtr<IDXGISwapChain> m_swapChain;
    ComPtr<IDXGISwapChain1> m_swapChain1;
    ComPtr<IDXGISwapChain2> m_swapChain2;
    ComPtr<IDXGISwapChain3> m_swapChain3;
    ComPtr<IDXGISwapChain4> m_swapChain4;
    UINT m_frameIndex;

    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    ComPtr<ID3D12DescriptorHeap> m_srvHeap;
    UINT m_rtvDescriptorSize;

    std::array<ComPtr<ID3D12Resource>, FrameCount> m_renderTargets;
    ComPtr<ID3D12CommandAllocator> m_commandAllocator;

    ComPtr<ID3D12RootSignature> m_rootSignature;
    ComPtr<ID3D12PipelineState> m_pipelineState;
    ComPtr<ID3D12GraphicsCommandList> m_commandList;
    ComPtr<ID3D12Resource> m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
    ComPtr<ID3D12Resource> m_texture;
    ComPtr<ID3D12Fence> m_fence;
    UINT64 m_fenceValue;
    HANDLE m_fenceEvent;

    CD3DX12_VIEWPORT m_viewport;
    CD3DX12_RECT m_scissorRect;
};

struct Vertex
{
    XMFLOAT3 position;
    XMFLOAT2 uv;
};

/**
 *	コンストラクタ
 */
App::Private::Private(App* app)
    : app(app)
{
}

/**
 *	デストラクタ
 */
App::Private::~Private()
{
}

/**
 *	レンダリングパイプラインをロード
 */
void App::Private::LoadPipeline()
{
    const auto [screenWidth, screenHeight] = app->GetScreenSize();

    LoadFactory();
    LoadAdapter();
    LoadDevice();

    D3D12_COMMAND_QUEUE_DESC queueDesc = { };
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.NodeMask = 0; // マルチGPUで使用する
    ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

    if (factory2 != nullptr)
    {
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { };
        swapChainDesc.Width = screenWidth;
        swapChainDesc.Height = screenHeight;
        swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.Stereo = FALSE;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = FrameCount;
        swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        swapChainDesc.Flags = static_cast<DXGI_SWAP_CHAIN_FLAG>(0);

        ComPtr<IDXGISwapChain1> swapChain;
        _In_  IUnknown* pDevice = m_commandQueue.Get();
        _In_  HWND hWnd = app->GetWindowsHandle();
        _In_  const DXGI_SWAP_CHAIN_DESC1* pDesc = &swapChainDesc;
        _In_opt_  const DXGI_SWAP_CHAIN_FULLSCREEN_DESC* pFullscreenDesc = nullptr;
        _In_opt_  IDXGIOutput* pRestrictToOutput = nullptr;
        _COM_Outptr_  IDXGISwapChain1** ppSwapChain = &swapChain;
        ThrowIfFailed(factory2->CreateSwapChainForHwnd(pDevice, hWnd, pDesc, pFullscreenDesc, pRestrictToOutput, ppSwapChain));

        ThrowIfFailed(factory2->MakeWindowAssociation(app->GetWindowsHandle(), DXGI_MWA_NO_ALT_ENTER));

        ThrowIfFailed(swapChain.As(&m_swapChain), { E_NOINTERFACE });
        ThrowIfFailed(swapChain.As(&m_swapChain2), { E_NOINTERFACE });
        ThrowIfFailed(swapChain.As(&m_swapChain3), { E_NOINTERFACE });
        ThrowIfFailed(swapChain.As(&m_swapChain4), { E_NOINTERFACE });
    }

    if (m_swapChain3 != nullptr)
    {
        m_frameIndex = m_swapChain3->GetCurrentBackBufferIndex();
    }

    // Create descriptor heaps.
    {
        // Describe and create a render target view (RTV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = { };
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.NumDescriptors = FrameCount;
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        rtvHeapDesc.NodeMask = 0;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

        // Describe and create a shader resource view (SRV) heap for the texture.
        D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = { };
        srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        srvHeapDesc.NumDescriptors = 1;
        srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        srvHeapDesc.NodeMask = 0;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap)));

        m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }

    // Create frame resources.
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

        // Create a RTV for each frame.
        for (UINT n = 0; n < FrameCount; n++)
        {
            ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));

            _In_opt_  ID3D12Resource* pResource = m_renderTargets[n].Get();
            _In_opt_  const D3D12_RENDER_TARGET_VIEW_DESC *pDesc = nullptr;
            _In_  D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor = rtvHandle;
            m_device->CreateRenderTargetView(pResource, pDesc, DestDescriptor);

            rtvHandle.Offset(1, m_rtvDescriptorSize);
        }
    }

    ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));

    m_viewport = CD3DX12_VIEWPORT(0.f, 0.f, (FLOAT)screenWidth, (FLOAT)screenHeight);
    m_scissorRect = CD3DX12_RECT(0, 0, screenWidth, screenHeight);
}

/**
 *
 */
void App::Private::LoadFactory()
{
    const auto Flags = ([]()
        {
            UINT ret = 0;

            ComPtr<ID3D12Debug> debugController;
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
            {
                debugController->EnableDebugLayer();
                ret |= DXGI_CREATE_FACTORY_DEBUG;

                ComPtr<ID3D12Debug1> debugController1;
                ComPtr<ID3D12Debug2> debugController2;
                ComPtr<ID3D12Debug3> debugController3;
                ComPtr<ID3D12Debug4> debugController4;
                ComPtr<ID3D12Debug5> debugController5;
                ThrowIfFailed(debugController.As(&debugController1), { E_NOINTERFACE });
                ThrowIfFailed(debugController.As(&debugController2), { E_NOINTERFACE });
                ThrowIfFailed(debugController.As(&debugController3), { E_NOINTERFACE });
                ThrowIfFailed(debugController.As(&debugController4), { E_NOINTERFACE });
                ThrowIfFailed(debugController.As(&debugController5), { E_NOINTERFACE });

                if (debugController3 != nullptr)
                {
                    debugController3->SetEnableGPUBasedValidation(TRUE);
                    debugController3->SetEnableSynchronizedCommandQueueValidation(TRUE);
                }
                else if (debugController1 != nullptr)
                {
                    debugController1->SetEnableGPUBasedValidation(TRUE);
                    debugController1->SetEnableSynchronizedCommandQueueValidation(TRUE);
                }

                if (debugController3 != nullptr)
                {
                    debugController3->SetGPUBasedValidationFlags(D3D12_GPU_BASED_VALIDATION_FLAGS_NONE);
                }
                else if (debugController2 != nullptr)
                {
                    debugController2->SetGPUBasedValidationFlags(D3D12_GPU_BASED_VALIDATION_FLAGS_NONE);
                }

                if (debugController5 != nullptr)
                {
                    debugController5->SetEnableAutoName(TRUE);
                }
            }

            return ret;
        })();

    ThrowIfFailed(CreateDXGIFactory2(Flags, IID_PPV_ARGS(&factory)));
    ThrowIfFailed(factory.As(&factory1), { E_NOINTERFACE });
    ThrowIfFailed(factory.As(&factory2), { E_NOINTERFACE });
    ThrowIfFailed(factory.As(&factory3), { E_NOINTERFACE });
    ThrowIfFailed(factory.As(&factory4), { E_NOINTERFACE });
    ThrowIfFailed(factory.As(&factory5), { E_NOINTERFACE });
    ThrowIfFailed(factory.As(&factory6), { E_NOINTERFACE });
    ThrowIfFailed(factory.As(&factory7), { E_NOINTERFACE });
}

/**
 *
 */
void App::Private::LoadAdapter()
{
    if (adapter == nullptr && factory6 != nullptr)
    {
        for (UINT Adapter = 0; ; Adapter++)
        {
            const auto [isFound, adapter, gpuReference] = ([&]() -> std::tuple<bool, ComPtr<IDXGIAdapter>, DXGI_GPU_PREFERENCE>
                {
                    for (const auto GpuPreference :
                        {
                            DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, // dGPU, xGPU
                            DXGI_GPU_PREFERENCE_MINIMUM_POWER, // iGPU
                            DXGI_GPU_PREFERENCE_UNSPECIFIED,
                        }
                    )
                    {
                        ComPtr<IDXGIAdapter> adapter;
                        if (SUCCEEDED(factory6->EnumAdapterByGpuPreference(Adapter, GpuPreference, IID_PPV_ARGS(&adapter))))
                        {
                            return { true, adapter, GpuPreference };
                        }
                    }

                    return { false, ComPtr<IDXGIAdapter>(), static_cast<DXGI_GPU_PREFERENCE>(-1) };
                })();
            if (!isFound)
            {
                break;
            }

            ComPtr<IDXGIAdapter1> adapter1;
            ComPtr<IDXGIAdapter2> adapter2;
            ComPtr<IDXGIAdapter3> adapter3;
            ComPtr<IDXGIAdapter4> adapter4;
            ThrowIfFailed(adapter.As(&adapter1), { E_NOINTERFACE });
            ThrowIfFailed(adapter.As(&adapter2), { E_NOINTERFACE });
            ThrowIfFailed(adapter.As(&adapter3), { E_NOINTERFACE });
            ThrowIfFailed(adapter.As(&adapter4), { E_NOINTERFACE });

            if (adapter4 != nullptr)
            {
                DXGI_ADAPTER_DESC3 Desc3;
                ThrowIfFailed(adapter4->GetDesc3(&Desc3));
                Log(std::source_location::current(), "Desc3 %d `%S`", Adapter, Desc3.Description);
                Log("\tVendorId = %u", Desc3.VendorId);
                Log("\tDeviceId = %u", Desc3.DeviceId);
                Log("\tSubSysId = %u", Desc3.SubSysId);
                Log("\tRevision = %u", Desc3.Revision);
                Log("\tDedicatedVideoMemory = %zuMiB", Desc3.DedicatedVideoMemory / 1024 / 1024);
                Log("\tDedicatedSystemMemory = %zuMiB", Desc3.DedicatedSystemMemory / 1024 / 1024);
                Log("\tSharedSystemMemory = %zuMiB", Desc3.SharedSystemMemory / 1024 / 1024);
                Log("\tAdapterLuid = %08X%08XH", Desc3.AdapterLuid.HighPart, Desc3.AdapterLuid.LowPart);
                Log("\tFlags = %sH", ToString(Desc3.Flags).c_str());
                Log("\tGraphicsPreemptionGranularity = %s", ToString(Desc3.GraphicsPreemptionGranularity).c_str());
                Log("\tComputePreemptionGranularity = %s", ToString(Desc3.ComputePreemptionGranularity).c_str());
            }

            DXGI_ADAPTER_DESC1 Desc1;
            ThrowIfFailed(adapter1->GetDesc1(&Desc1));
            if ((Desc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0)
            {
                if (this->adapter == nullptr)
                {
                    this->adapter = adapter;
                    this->adapter1 = adapter1;
                    this->adapter2 = adapter2;
                    this->adapter3 = adapter3;
                    this->adapter4 = adapter4;
                    this->gpuReference = gpuReference;
                }
            }
        }
    }

    if (adapter == nullptr && factory1 != nullptr)
    {
        for (UINT Adapter = 0; ; Adapter++)
        {
            ThrowIfFailed(factory1->EnumAdapters1(Adapter, &adapter1));

            DXGI_ADAPTER_DESC1 Desc1;
            ThrowIfFailed(adapter1->GetDesc1(&Desc1));
            if ((Desc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0)
            {
                ThrowIfFailed(adapter1.As(&adapter), { E_NOINTERFACE });
                ThrowIfFailed(adapter1.As(&adapter2), { E_NOINTERFACE });
                ThrowIfFailed(adapter1.As(&adapter3), { E_NOINTERFACE });
                ThrowIfFailed(adapter1.As(&adapter4), { E_NOINTERFACE });
                break;
            }
        }
    }

    if (adapter == nullptr)
    {
        throw new std::exception;
    }
}

/**
 *
 */
void App::Private::LoadDevice()
{
    for (const auto level :
        {
            D3D_FEATURE_LEVEL_12_2,
            D3D_FEATURE_LEVEL_12_1,
            D3D_FEATURE_LEVEL_12_0,
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_3,
            D3D_FEATURE_LEVEL_9_2,
            D3D_FEATURE_LEVEL_9_1,
            D3D_FEATURE_LEVEL_1_0_CORE,
        }
    )
    {
        if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), level, IID_PPV_ARGS(&m_device))))
        {
            const auto name = StringGenerator::SPrintf(
                L"%S(%d,%d): device",
                std::source_location::current().file_name(),
                std::source_location::current().line(),
                std::source_location::current().column()
            );
            m_device->SetName(name.c_str());
            MinimumFeatureLevel = level;
            return;
        }
    }
}

/**
 *
 */
void App::Private::LoadAssets()
{
    // Create the root signature.
    {
        D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

        // This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

        if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
        {
            featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
        }

        std::array<CD3DX12_DESCRIPTOR_RANGE1, 1> ranges;
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

        CD3DX12_ROOT_PARAMETER1 rootParameters[1];
        rootParameters[0].InitAsDescriptorTable(static_cast<UINT>(ranges.size()), ranges.data(), D3D12_SHADER_VISIBILITY_PIXEL);

        D3D12_STATIC_SAMPLER_DESC sampler = {};
        sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.MipLODBias = 0;
        sampler.MaxAnisotropy = 0;
        sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        sampler.MinLOD = 0.0f;
        sampler.MaxLOD = D3D12_FLOAT32_MAX;
        sampler.ShaderRegister = 0;
        sampler.RegisterSpace = 0;
        sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
        ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
    }

    // Create the pipeline state, which includes compiling and loading shaders.
    {
        ComPtr<ID3DBlob> vertexShader;
        ComPtr<ID3DBlob> pixelShader;

#if defined(_DEBUG)
        // Enable better shader debugging with the graphics debugging tools.
        UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        UINT compileFlags = 0;
#endif

        ThrowIfFailed(D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
        ThrowIfFailed(D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));

        // Define the vertex input layout.
        D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };

        // Describe and create the graphics pipeline state object (PSO).
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
        psoDesc.pRootSignature = m_rootSignature.Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.DepthEnable = FALSE;
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;
        ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
    }

    // Create the command list.
    ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList)));

    // Create the vertex buffer.
    {
        // Define the geometry for a triangle.
        Vertex triangleVertices[] =
        {
            { { 0.0f, 0.25f, 0.0f }, { 0.5f, 0.0f } },
            { { 0.25f, -0.25f, 0.0f }, { 1.0f, 1.0f } },
            { { -0.25f, -0.25f, 0.0f }, { 0.0f, 1.0f } }
        };

        const UINT vertexBufferSize = sizeof(triangleVertices);

        // Note: using upload heaps to transfer static data like vert buffers is not 
        // recommended. Every time the GPU needs it, the upload heap will be marshalled 
        // over. Please read up on Default Heap usage. An upload heap is used here for 
        // code simplicity and because there are very few verts to actually transfer.
        D3D12_HEAP_PROPERTIES HeapProperties;
        D3D12_RESOURCE_DESC Desc;
        ThrowIfFailed(m_device->CreateCommittedResource(
            &(HeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD)),
            D3D12_HEAP_FLAG_NONE,
            &(Desc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize)),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&m_vertexBuffer)));

        // Copy the triangle data to the vertex buffer.
        UINT8* pVertexDataBegin;
        CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
        ThrowIfFailed(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
        memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
        m_vertexBuffer->Unmap(0, nullptr);

        // Initialize the vertex buffer view.
        m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
        m_vertexBufferView.StrideInBytes = sizeof(Vertex);
        m_vertexBufferView.SizeInBytes = vertexBufferSize;
    }

    // Note: ComPtr's are CPU objects but this resource needs to stay in scope until
    // the command list that references it has finished executing on the GPU.
    // We will flush the GPU at the end of this method to ensure the resource is not
    // prematurely destroyed.
    ComPtr<ID3D12Resource> textureUploadHeap;

    // Create the texture.
    {
        // Describe and create a Texture2D.
        D3D12_RESOURCE_DESC textureDesc = {};
        textureDesc.MipLevels = 1;
        textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        textureDesc.Width = TextureWidth;
        textureDesc.Height = TextureHeight;
        textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
        textureDesc.DepthOrArraySize = 1;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

        D3D12_HEAP_PROPERTIES HeapProperties;
        ThrowIfFailed(m_device->CreateCommittedResource(
            &(HeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT)),
            D3D12_HEAP_FLAG_NONE,
            &textureDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&m_texture)));

        const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_texture.Get(), 0, 1);

        // Create the GPU upload buffer.
        D3D12_HEAP_PROPERTIES HeapProperties2;
        D3D12_RESOURCE_DESC Desc;
        ThrowIfFailed(m_device->CreateCommittedResource(
            &(HeapProperties2 = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD)),
            D3D12_HEAP_FLAG_NONE,
            &(Desc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize)),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&textureUploadHeap)));

        // Copy data to the intermediate upload heap and then schedule a copy 
        // from the upload heap to the Texture2D.
        std::vector<UINT8> texture = GenerateTextureData(TextureWidth, TextureWidth, TexturePixelSize);

        D3D12_SUBRESOURCE_DATA textureData = {};
        textureData.pData = &texture[0];
        textureData.RowPitch = TextureWidth * TexturePixelSize;
        textureData.SlicePitch = textureData.RowPitch * TextureHeight;

        UpdateSubresources(m_commandList.Get(), m_texture.Get(), textureUploadHeap.Get(), 0, 0, 1, &textureData);
        D3D12_RESOURCE_BARRIER Barriers;
        m_commandList->ResourceBarrier(1, &(Barriers = CD3DX12_RESOURCE_BARRIER::Transition(m_texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)));

        // Describe and create a SRV for the texture.
        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = textureDesc.Format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        m_device->CreateShaderResourceView(m_texture.Get(), &srvDesc, m_srvHeap->GetCPUDescriptorHandleForHeapStart());
    }

    // Close the command list and execute it to begin the initial GPU setup.
    ThrowIfFailed(m_commandList->Close());
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Create synchronization objects and wait until assets have been uploaded to the GPU.
    {
        ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
        m_fenceValue = 1;

        // Create an event handle to use for frame synchronization.
        m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (m_fenceEvent == nullptr)
        {
            ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
        }

        // Wait for the command list to execute; we are reusing the same command 
        // list in our main loop but for now, we just want to wait for setup to 
        // complete before continuing.
        WaitForPreviousFrame();
    }
}

/**
 *	
 */
void App::Private::PopulateCommandList()
{
    // Command list allocators can only be reset when the associated 
    // command lists have finished execution on the GPU; apps should use 
    // fences to determine GPU execution progress.
    ThrowIfFailed(m_commandAllocator->Reset());

    // However, when ExecuteCommandList() is called on a particular command 
    // list, that command list can then be reset at any time and must be before 
    // re-recording.
    ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get()));

    // Set necessary state.
    m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

    ID3D12DescriptorHeap* ppHeaps[] = { m_srvHeap.Get() };
    m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

    m_commandList->SetGraphicsRootDescriptorTable(0, m_srvHeap->GetGPUDescriptorHandleForHeapStart());
    m_commandList->RSSetViewports(1, &m_viewport);
    m_commandList->RSSetScissorRects(1, &m_scissorRect);

    // Indicate that the back buffer will be used as a render target.
    D3D12_RESOURCE_BARRIER Barriers;
    m_commandList->ResourceBarrier(1, &(Barriers = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET)));

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    // Record commands.
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    m_commandList->DrawInstanced(3, 1, 0, 0);

    // Indicate that the back buffer will now be used to present.
    D3D12_RESOURCE_BARRIER Barriers2;
    m_commandList->ResourceBarrier(1, &(Barriers2 = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT)));

    ThrowIfFailed(m_commandList->Close());
}

/**
 *
 */
void App::Private::WaitForPreviousFrame()
{
    // WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
    // This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
    // sample illustrates how to use fences for efficient resource usage and to
    // maximize GPU utilization.

    // Signal and increment the fence value.
    const UINT64 fence = m_fenceValue;
    ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), fence));
    m_fenceValue++;

    // Wait until the previous frame is finished.
    if (m_fence->GetCompletedValue() < fence)
    {
        ThrowIfFailed(m_fence->SetEventOnCompletion(fence, m_fenceEvent));
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }

    m_frameIndex = m_swapChain3->GetCurrentBackBufferIndex();
}

std::vector<UINT8> GenerateTextureData(int width, int height, int pixelSize)
{
    const UINT rowPitch = width * pixelSize;
    const UINT cellPitch = rowPitch >> 3;        // The width of a cell in the checkboard texture.
    const UINT cellHeight = width >> 3;    // The height of a cell in the checkerboard texture.
    const UINT textureSize = rowPitch * height;

    std::vector<UINT8> data(textureSize);
    UINT8* pData = &data[0];

    for (UINT n = 0; n < textureSize; n += pixelSize)
    {
        UINT x = n % rowPitch;
        UINT y = n / rowPitch;
        UINT i = x / cellPitch;
        UINT j = y / cellHeight;

        if (i % 2 == j % 2)
        {
            pData[n] = 0x00;        // R
            pData[n + 1] = 0x00;    // G
            pData[n + 2] = 0x00;    // B
            pData[n + 3] = 0xff;    // A
        }
        else
        {
            pData[n] = 0xff;        // R
            pData[n + 1] = 0xff;    // G
            pData[n + 2] = 0xff;    // B
            pData[n + 3] = 0xff;    // A
        }
    }

    return data;
}
