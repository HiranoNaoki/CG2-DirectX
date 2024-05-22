#include <Windows.h>
#include<cstdint>
#include<string>
#include<format>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<cassert>
#include <dxgidebug.h>
#include <dxcapi.h>


#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dxcompiler.lib")

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")



LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);

}

struct Vector4 final
{
	float x;
	float y;
	float z;
	float w;
};




std::wstring ConvertString(const std::string& str) {
	if (str.empty()) {
		return std::wstring();
	}

	auto sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), NULL, 0);
	if (sizeNeeded == 0) {
		return std::wstring();
	}
	std::wstring result(sizeNeeded, 0);
	MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), &result[0], sizeNeeded);
	return result;
}

std::string ConvertString(const std::wstring& str) {
	if (str.empty()) {
		return std::string();
	}

	auto sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0, NULL, NULL);
	if (sizeNeeded == 0) {
		return std::string();
	}
	std::string result(sizeNeeded, 0);
	WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), sizeNeeded, NULL, NULL);
	return result;
}




void Log(const std::string& message) {
	OutputDebugStringA(message.c_str());
}


IDxcBlob* CompileShader(
	const std::wstring& filePath,
	const wchar_t* profile,
	IDxcUtils* dxcUtils,
	IDxcCompiler3* dxCompiler,
	IDxcIncludeHandler* includeHandler)
{
	Log(ConvertString(std::format(L"Begin CompileShader, path:{}, profile:{}\n", filePath, profile)));

	IDxcBlobEncoding* ShaderSource = nullptr;

	HRESULT hr = dxcUtils->LoadFile(filePath.c_str(), NULL, &ShaderSource);

	assert(SUCCEEDED(hr));

	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = ShaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = ShaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;



	LPCWSTR arguments[] = {
	filePath.c_str(),
	L"-E",L"main",
	L"-T",profile,
	L"-Zi",L"-Qembed_debug",
	L"-Od",
	L"-Zpr",
	};

	IDxcResult* shaderResult = nullptr;
	hr = dxCompiler->Compile(
		&shaderSourceBuffer,
		arguments,
		_countof(arguments),
		includeHandler,
		IID_PPV_ARGS(&shaderResult)
	);

	assert(SUCCEEDED(hr));

	IDxcBlobUtf8* shaderError = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
		Log(shaderError->GetStringPointer());

		assert(false);
	}
		IDxcBlob* shaderBlob = nullptr;
		hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
		assert(SUCCEEDED(hr));

		Log(ConvertString(std::format(L"Compile Suceeded,path:{},profile:{}\n", filePath, profile)));

		ShaderSource->Release();
		shaderResult->Release();

		return shaderBlob;
	
}

ID3D12Resource* CreateBufferResource(ID3D12Device* device, size_t sizeInBytes) {


	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC vertexResoureDesc{};

	vertexResoureDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexResoureDesc.Width = sizeInBytes;

	vertexResoureDesc.Height = 1;
	vertexResoureDesc.DepthOrArraySize = 1;
	vertexResoureDesc.MipLevels = 1;
	vertexResoureDesc.SampleDesc.Count = 1;

	vertexResoureDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	ID3D12Resource* resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
		&vertexResoureDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));
	return resource;
}




int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

#pragma region window
	WNDCLASS wc{};
	wc.lpfnWndProc = WindowProc;
	wc.lpszClassName = L"CG2WindowDlass";
	wc.hInstance = GetModuleHandle(nullptr);
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);


	RegisterClass(&wc);

	
	const int32_t kClientWidth = 1280;
	const int32_t kClientHeight = 720;

	RECT wrc = { 0,0,kClientWidth,kClientHeight };

	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	HWND hwnd = CreateWindow(
		wc.lpszClassName,
		L"CG2",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,
		wrc.bottom - wrc.top,
		nullptr,
		nullptr,
		wc.hInstance,
		nullptr);

	ShowWindow(hwnd, SW_SHOW);

#pragma endregion 


#ifdef _DEBUG
	ID3D12Debug1* debugController = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		debugController->EnableDebugLayer();
		debugController->SetEnableGPUBasedValidation(TRUE);
	}

#endif 




#pragma region factory
	IDXGIFactory7* dxgifactory = nullptr;

	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgifactory));

	assert(SUCCEEDED(hr));

#pragma endregion

#pragma region adapter
	IDXGIAdapter4* useAdapter = nullptr;

	for (UINT i = 0; dxgifactory -> EnumAdapterByGpuPreference(i,
		DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,IID_PPV_ARGS(&useAdapter)) !=
		DXGI_ERROR_NOT_FOUND; ++i){

		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useAdapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr));

		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE))
		{
			Log(ConvertString(std::format(L"use adapter : {}\n", adapterDesc.Description)));
			break;
		}
		useAdapter = nullptr;
	}
	assert(useAdapter != nullptr);
#pragma endregion

#pragma region device

	ID3D12Device* device = nullptr;


	

	assert(SUCCEEDED(hr));

	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0
	};

	const char* featureLevelStrings[] = { "12.2","12.1","12.0"};

	for (size_t i = 0; i < _countof(featureLevels); ++i) {
		hr = D3D12CreateDevice(useAdapter, featureLevels[i], IID_PPV_ARGS(&device));
	
		if (SUCCEEDED(hr)) {
			Log(std::format("FeatureLevel : {}\n", featureLevelStrings[i]));
			break;
		}
	}



	assert(device != nullptr);
	Log("Complete create D312Devaice!!!\n");
#pragma endregion

#ifdef _DEBUG
	ID3D12InfoQueue* infoQueue = nullptr;
	if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);

		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);

		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);


		D3D12_MESSAGE_ID denyIds[] = {
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};

		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };

		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;

		infoQueue->PushStorageFilter(&filter);


		infoQueue->Release();
	}
#endif 


#pragma region CommndQueue
	ID3D12CommandQueue* commandQueue = nullptr;
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	hr = device->CreateCommandQueue(&commandQueueDesc,
		IID_PPV_ARGS(&commandQueue));
#pragma endregion

#pragma region Allocator
	ID3D12CommandAllocator* commandAllocator = nullptr;
	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,IID_PPV_ARGS(&commandAllocator));
	assert(SUCCEEDED(hr));
#pragma endregion 

#pragma region List
	ID3D12GraphicsCommandList* commandList = nullptr;
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, nullptr,
		IID_PPV_ARGS(&commandList));


	assert(SUCCEEDED(hr));
#pragma endregion

#pragma region Swapchain
	IDXGISwapChain4* swapChain = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = kClientWidth;
	swapChainDesc.Height = kClientHeight;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	hr = dxgifactory->CreateSwapChainForHwnd(commandQueue, hwnd, &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(&swapChain));
	assert(SUCCEEDED(hr));
#pragma endregion

#pragma region DescriptorHeap
	ID3D12DescriptorHeap* rtvDescriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc{};
	rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvDescriptorHeapDesc.NumDescriptors = 2;
	hr = device->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap));

	assert(SUCCEEDED(hr));
#pragma endregion


	


	ID3D12Resource* swapChainResources[2] = { nullptr };
	hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));
	assert(SUCCEEDED(hr));
	hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
	assert(SUCCEEDED(hr));


	
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle[2];

	rtvHandle[0] = rtvStartHandle;
	device->CreateRenderTargetView(swapChainResources[0], &rtvDesc, rtvHandle[0]);
	rtvHandle[1].ptr = rtvHandle[0].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	device->CreateRenderTargetView(swapChainResources[1], &rtvDesc, rtvHandle[1]);

	MSG msg{};

	ID3D12Fence* fence = nullptr;
	uint64_t fenceValue = 0;
	hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	assert(SUCCEEDED(hr));

	HANDLE fenceEvent = CreateEvent(NULL, false, false, NULL);
	assert(fenceEvent != nullptr);

	IDxcUtils* dxcutils = nullptr;
	IDxcCompiler3* dxcCompiler = nullptr;
	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcutils));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	assert(SUCCEEDED(hr));

	IDxcIncludeHandler* includeHandler = nullptr;
	hr = dxcutils->CreateDefaultIncludeHandler(&includeHandler);
	assert(SUCCEEDED(hr));

	//Rootsignature
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_ROOT_PARAMETER rootParameters[1] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[0].Descriptor.ShaderRegister = 0;
	descriptionRootSignature.pParameters = rootParameters;
	descriptionRootSignature.NumParameters = _countof(rootParameters);

	ID3DBlob* sigunatureBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &sigunatureBlob, &errorBlob);
	if (FAILED(hr)) {
		Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	ID3D12RootSignature* rootSignature = nullptr;
	hr = device->CreateRootSignature(0, sigunatureBlob->GetBufferPointer(), sigunatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));

	//InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[1] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	D3D12_INPUT_LAYOUT_DESC inputLayouDesc{};
	inputLayouDesc.pInputElementDescs = inputElementDescs;
	inputLayouDesc.NumElements = _countof(inputElementDescs);

	
	D3D12_BLEND_DESC blendDesc{};

	blendDesc. RenderTarget[0].RenderTargetWriteMask =
		D3D12_COLOR_WRITE_ENABLE_ALL;
	
	D3D12_RASTERIZER_DESC rasterizerDesc{};

	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	
	IDxcBlob* vertexShaderBlob = CompileShader(L"Object3d.VS.hlsl",
		L"vs_6_0", dxcutils, dxcCompiler, includeHandler);
	assert(vertexShaderBlob != nullptr);

	IDxcBlob* pixelShaderBlob = CompileShader(L"Object3d.PS.hlsl",
		L"ps_6_0", dxcutils, dxcCompiler, includeHandler);
	assert(pixelShaderBlob != nullptr);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPilelineStateDesc{};
	graphicsPilelineStateDesc.pRootSignature = rootSignature;
	graphicsPilelineStateDesc.InputLayout = inputLayouDesc;
	graphicsPilelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),
	vertexShaderBlob->GetBufferSize() };//vertexShader
	graphicsPilelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),
	pixelShaderBlob->GetBufferSize() };
	graphicsPilelineStateDesc.BlendState = blendDesc;
	graphicsPilelineStateDesc.RasterizerState = rasterizerDesc;

	graphicsPilelineStateDesc.NumRenderTargets = 1;
	graphicsPilelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	graphicsPilelineStateDesc.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	graphicsPilelineStateDesc.SampleDesc.Count = 1;
	graphicsPilelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	ID3D12PipelineState* graphicsPipelinestate = nullptr;
	hr = device->CreateGraphicsPipelineState(&graphicsPilelineStateDesc,
		IID_PPV_ARGS(&graphicsPipelinestate));
	assert(SUCCEEDED(hr));

	ID3D12Resource* vertexResource = CreateBufferResource(device, sizeof(Vector4) * 3);

	D3D12_VERTEX_BUFFER_VIEW vertexBufferview{};

	vertexBufferview.BufferLocation = vertexResource->GetGPUVirtualAddress();

	vertexBufferview.SizeInBytes = sizeof(Vector4) * 3;

	vertexBufferview.StrideInBytes = sizeof(Vector4);

	Vector4* vertexDate = nullptr;

	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexDate));

	vertexDate[0] = { -0.5f,-0.5,0.0f,1.0f };

	vertexDate[1] = { 0.0f,0.5f,0.0f,1.0f };

	vertexDate[2] = { 0.5f,-0.5f,0.0f,1.0f };

	D3D12_VIEWPORT viewport{};

	viewport.Width = kClientWidth;
	viewport.Height = kClientHeight;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	D3D12_RECT scissorRect{};
	scissorRect.left = 0;
	scissorRect.right = kClientWidth;
	scissorRect.top = 0;
	scissorRect.bottom = kClientHeight;

	ID3D12Resource* materialResource = CreateBufferResource(device, sizeof(Vector4));

	Vector4* materialDate = nullptr;
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialDate));

	*materialDate = Vector4(1.0f, 0.0f, 0.0f, 1.0f);

	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();

			D3D12_RESOURCE_BARRIER barrier{};

			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;

			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.pResource = swapChainResources[backBufferIndex];
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
			commandList->ResourceBarrier(1, &barrier);



			commandList->OMSetRenderTargets(1, &rtvHandle[backBufferIndex], false, nullptr);

			float clearColor[] = { 0.1f,0.25f,0.5f,1.0f, };
			commandList->ClearRenderTargetView(rtvHandle[backBufferIndex],clearColor,0,nullptr);
		
			commandList->RSSetViewports(1, &viewport);
			commandList->RSSetScissorRects(1, &scissorRect);

			commandList->SetGraphicsRootSignature(rootSignature);
			commandList->SetPipelineState(graphicsPipelinestate);
			commandList->IASetVertexBuffers(0, 1, &vertexBufferview);

			commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());

			commandList->DrawInstanced(3, 1, 0, 0);

			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

			commandList->ResourceBarrier(1, &barrier);

			hr = commandList->Close();
			assert(SUCCEEDED(hr));

			ID3D12CommandList* commandLists[] = { commandList };
			commandQueue->ExecuteCommandLists(1, commandLists);
			swapChain->Present(1, 0);


			fenceValue++;

			commandQueue->Signal(fence, fenceValue);
			if (fence->GetCompletedValue()<fenceValue)
			{
				fence->SetEventOnCompletion(fenceValue, fenceEvent);
				WaitForSingleObject(fenceEvent, INFINITE);
			}

			hr = commandAllocator->Reset();
			assert(SUCCEEDED(hr));
			hr = commandList->Reset(commandAllocator, nullptr);
			assert(SUCCEEDED(hr));
		}

	}
	

	OutputDebugStringA("Hello,DiretX!\n");

	CloseHandle(fenceEvent);
	fence->Release();
	rtvDescriptorHeap->Release();
	swapChainResources[0]->Release();
	swapChainResources[1]->Release();
	swapChain->Release();
	commandList->Release();
	commandAllocator->Release();
	commandQueue->Release();
	device->Release();
	useAdapter->Release();
	dxgifactory->Release();


	vertexResource->Release();
	graphicsPipelinestate->Release();
	sigunatureBlob->Release();
	if (errorBlob) {
		errorBlob->Release();
	}
	rootSignature->Release();
	pixelShaderBlob->Release();
	vertexShaderBlob->Release();

	materialResource->Release();







#ifdef _DEBUG
	debugController->Release();
#endif
	CloseWindow(hwnd);

	
	IDXGIDebug1* debug;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
		debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
		debug->Release();
	}

	return 0;
}