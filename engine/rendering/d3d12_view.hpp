#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <d3d12sdklayers.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <array>
#include <atomic>
#include <span>

namespace engine {
struct Vertex {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 color;
};

// One window and one immutable triangle buffer. Owns the GPU lifetime as a unit.
struct D3D12View {
    D3D12View() = default;
    ~D3D12View();
    D3D12View(const D3D12View&) = delete;
    D3D12View& operator=(const D3D12View&) = delete;
    void initialize(HWND window, UINT width, UINT height, std::span<const Vertex> vertices);
    void resize(UINT width, UINT height);
    void draw(const DirectX::XMFLOAT4X4& view_projection);

private:
    void wait_for_gpu();
    void create_targets();
    Microsoft::WRL::ComPtr<ID3D12Device> device;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> queue;
    Microsoft::WRL::ComPtr<IDXGISwapChain3> swap_chain;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtv_heap, dsv_heap;
    std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> back_buffers;
    Microsoft::WRL::ComPtr<ID3D12Resource> depth, vertex_buffer;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commands;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> root_signature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipeline;
    Microsoft::WRL::ComPtr<ID3D12Fence> fence;
    HANDLE fence_event = nullptr;
    UINT64 fence_value = 0, frame_count = 0;
    UINT width = 0, height = 0, rtv_stride = 0, vertex_count = 0;
    D3D12_VERTEX_BUFFER_VIEW vertex_view{};
#ifdef _DEBUG
    Microsoft::WRL::ComPtr<ID3D12InfoQueue1> info_queue;
    DWORD callback_cookie = 0;
    bool callback_registered = false;
    std::atomic_uint validation_errors = 0;
#endif
};
}
