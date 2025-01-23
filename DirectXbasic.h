#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include "WinApp.h"
#include <array>
#include <dxcapi.h>
#include <string>
#include "externals/DirectXTex/DirectXTex.h"



class DirectXbasic {

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

	Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(const std::wstring& filePath, const wchar_t* profile);

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeInBytes);

	Microsoft::WRL::ComPtr <ID3D12Resource>CreateTextureResource(Microsoft::WRL::ComPtr <ID3D12Device> device, const DirectX::TexMetadata& metadata);

	static DirectX::ScratchImage LoadTexture(const std::string& filePath);


public:

	D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUDescriptorHandle(uint32_t index);

	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUDescriptorHandle(uint32_t index);


	void Intialize(WinApp* winApp);

	void Device();


	void PreDraw();
	void PostDraw();



	ID3D12Device* GetDevice() const { return device.Get(); }
	ID3D12GraphicsCommandList* GetCommandList() const { return commandList.Get(); }
private:

	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);

	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);



	void Command();

	void Swap();

	void DepthBuffer();

	void DescriptorHeap();

	void RenderTargetView();


	void DepthStencil();

	void Fence();

	void Viewport();

	void ScissorRect();

	void DxccomPtr();

	void ImGUI();

	Microsoft::WRL::ComPtr<ID3D12Device> device;

	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory;


	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);

	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory = nullptr;

	Microsoft::WRL::ComPtr < ID3D12Device> device = nullptr;


	Microsoft::WRL::ComPtr <ID3D12CommandQueue> commandQueue = nullptr;


	Microsoft::WRL::ComPtr <ID3D12CommandAllocator> commandAllocator = nullptr;


	Microsoft::WRL::ComPtr <ID3D12GraphicsCommandList> commandList = nullptr;

	WinApp* winApp = nullptr;

	Microsoft::WRL::ComPtr <IDXGISwapChain4> swapChain = nullptr;

	uint32_t descriptorSizeSRV = 0;
	uint32_t descriptorSizeRTV = 0;
	uint32_t descriptorSizeDSV = 0;

	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> rtvDescriptorHeap = nullptr;


	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap = nullptr;


	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> dsvDescriptorHeap = nullptr;

	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> swapChainResources;

	D3D12_CPU_DESCRIPTOR_HANDLE	rtvHandles[2];


	Microsoft::WRL::ComPtr <ID3D12Fence> fence = nullptr;

	UINT64 fenceValue = 0;


	HANDLE fenceEvent = nullptr;

	D3D12_VIEWPORT viewport{};

	D3D12_RECT scissorRect{};


	IDxcUtils* dxcUtils = nullptr;

	IDxcCompiler3* dxcCompiler = nullptr;


	IDxcIncludeHandler* includeHandler = nullptr;




};