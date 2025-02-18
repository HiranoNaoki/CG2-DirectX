#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include "WinApp.h"
#include <array>
#include <dxcapi.h>
#include <string>
#include "externals/DirectXTex/DirectXTex.h"
#include"externals/imgui/imgui_impl_dx12.h"
#include"externals/imgui/imgui_impl_win32.h"
#include<chrono>

#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dxcompiler.lib")

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")


class DirectXbasic {



public:

	void Initialize(WinApp* winApp);

	void Device();

	void Command();

	void Swap();

	void DepthBuffer();

	void DescriptorHeap();


	


	void RenderTargetView();


	void DepthStencil();

	void Fence();

	void Viewport();

	void ScissorRect();

	void dxcCompilerGenerate();

	void ImGUI();


	

	


	void PreDraw();
	void PostDraw();

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, int32_t width, int32_t height);


	

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

	Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(const std::wstring& filePath, const wchar_t* profile);

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeInBytes);

	Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureDate(Microsoft::WRL::ComPtr<ID3D12Resource> texture, const DirectX::ScratchImage& mipImges);

	Microsoft::WRL::ComPtr <ID3D12Resource>CreateTextureResource(Microsoft::WRL::ComPtr <ID3D12Device> device, const DirectX::TexMetadata& metadata);

	static DirectX::ScratchImage LoadTexture(const std::string& filePath);

	/* [[nodiscard]]
	Microsoft::WRL::ComPtr<ID3D12Resource>  UploadTextureDate(ID3D12Resource* texture, const DirectX::ScratchImage& mipimages);
	*/

	ID3D12Device* GetDevice() const { return device.Get(); }
	ID3D12GraphicsCommandList* GetCommandList() const { return commandList.Get(); }

	static  D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index) {

	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize * index);
	return handleCPU;
}

    static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index) {
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize * index);
	return handleGPU;
}

	D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUDescriptorHandle(uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUDescriptorHandle(uint32_t index);
	// RTV
	D3D12_CPU_DESCRIPTOR_HANDLE GetRTVCPUDescriptorHandle(uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetRTVGPUDescriptorHandle(uint32_t index);
	// DSV
	D3D12_CPU_DESCRIPTOR_HANDLE GetDSVCPUDescriptorHandle(uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetDSVGPUDescriptorHandle(uint32_t index);



private:

	

	uint32_t descriptorSizeSRV = 0;
	uint32_t descriptorSizeRTV = 0;
	uint32_t descriptorSizeDSV = 0;

	Microsoft::WRL::ComPtr < ID3D12Device> device;

	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory;

	
	
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};



	HRESULT hr;// = CoInitializeEx(0, COINIT_MULTITHREADED);

	

	


	Microsoft::WRL::ComPtr <ID3D12CommandQueue> commandQueue;
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};

	Microsoft::WRL::ComPtr <ID3D12CommandAllocator> commandAllocator;


	Microsoft::WRL::ComPtr <ID3D12GraphicsCommandList> commandList;
	

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};

	Microsoft::WRL::ComPtr <IDXGISwapChain4> swapChain;

	

	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> rtvDescriptorHeap;


	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap;


	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> dsvDescriptorHeap;

	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> swapChainResources;

	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle;

	D3D12_CPU_DESCRIPTOR_HANDLE	rtvHandles[2];



	


	Microsoft::WRL::ComPtr<ID3D12Debug1> debugController;

	
	Microsoft::WRL::ComPtr <ID3D12Fence> fence;

	HANDLE fenceEvent;
	UINT64 fenceValue = 0;

	



	Microsoft::WRL::ComPtr<ID3D12Resource> resource;

	D3D12_RECT scissorRect{};

		D3D12_VIEWPORT viewport{};


	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler;


	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource;

	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};

	D3D12_CLEAR_VALUE depthClearValue{};

	D3D12_HEAP_PROPERTIES heapProperties{  };

	D3D12_RESOURCE_DESC resourceDesc{};

	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils;

	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler;

		WinApp* winApp = nullptr;


		void InitializeFixFP5();

	void UpdateFixFP5();

	std::chrono::steady_clock::time_point reference_;
};