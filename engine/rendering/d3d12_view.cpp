#include "d3d12_view.hpp"
#include "scene_vs.h"
#include "scene_ps.h"
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <string>
#include <chrono>
#include <limits>

using Microsoft::WRL::ComPtr;
namespace engine {
namespace {
void check(HRESULT result, const char* operation)
{
    if (FAILED(result)) {
        char code[16];
        std::snprintf(code, sizeof(code), "0x%08lX", static_cast<unsigned long>(result));
        throw std::runtime_error(std::string(operation) + ": " + code);
    }
}
D3D12_RESOURCE_BARRIER transition(ID3D12Resource* resource,
    D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
{
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition = {resource, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, before, after};
    return barrier;
}
}

void D3D12View::initialize(HWND window, UINT initial_width, UINT initial_height,
    std::span<const Vertex> vertices, UINT translated_start, UINT overlay_start, UINT capacity)
{
    if (translated_start > vertices.size() || translated_start % 3 != 0)
        throw std::runtime_error("Invalid translated triangle range.");
    if (overlay_start < translated_start || overlay_start > vertices.size() || overlay_start % 3 != 0)
        throw std::runtime_error("Invalid overlay triangle range.");
    if (capacity % 3 != 0 || capacity > std::numeric_limits<UINT>::max()/sizeof(Vertex))
        throw std::runtime_error("Invalid dynamic triangle capacity.");
    dynamic_capacity=capacity;
    overlay_vertex_start = overlay_start;
    translated_vertex_start = translated_start;
    width = initial_width;
    height = initial_height;
    UINT factory_flags = 0;
#ifdef _DEBUG
    ComPtr<ID3D12Debug1> debug;
    check(D3D12GetDebugInterface(IID_PPV_ARGS(&debug)), "D3D12 debug layer (install Graphics Tools if absent)");
    debug->EnableDebugLayer();
    debug->SetEnableGPUBasedValidation(TRUE);
    factory_flags = DXGI_CREATE_FACTORY_DEBUG;
    std::fprintf(stderr, "D3D12 debug layer and GPU-based validation enabled.\n");
#endif
    ComPtr<IDXGIFactory6> factory;
    check(CreateDXGIFactory2(factory_flags, IID_PPV_ARGS(&factory)), "CreateDXGIFactory2");
    ComPtr<IDXGIAdapter1> adapter;
    for (UINT index = 0; ; ++index) {
        const HRESULT result = factory->EnumAdapterByGpuPreference(index,
            DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter));
        if (result == DXGI_ERROR_NOT_FOUND) break;
        check(result, "EnumAdapterByGpuPreference");
        DXGI_ADAPTER_DESC1 desc{};
        check(adapter->GetDesc1(&desc), "GetDesc1");
        if (!(desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) &&
            SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)))) {
            std::fprintf(stderr, "D3D12 adapter: %ls\n", desc.Description);
            break;
        }
        adapter.Reset();
    }
    if (!device) throw std::runtime_error("No hardware D3D12 adapter available.");
#ifdef _DEBUG
    check(device.As(&info_queue), "ID3D12InfoQueue1");
    check(info_queue->RegisterMessageCallback([](D3D12_MESSAGE_CATEGORY, D3D12_MESSAGE_SEVERITY severity,
        D3D12_MESSAGE_ID id, LPCSTR text, void* context) {
        auto& errors = *static_cast<std::atomic_uint*>(context);
        if (severity <= D3D12_MESSAGE_SEVERITY_ERROR) ++errors;
        std::fprintf(stderr, "D3D12 [%d, id %d]: %s\n", severity, id, text);
    }, D3D12_MESSAGE_CALLBACK_FLAG_NONE, &validation_errors, &callback_cookie), "RegisterMessageCallback");
    callback_registered = true;
#endif
    D3D12_COMMAND_QUEUE_DESC queue_desc{};
    check(device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&queue)), "CreateCommandQueue");
    DXGI_SWAP_CHAIN_DESC1 swap_desc{};
    swap_desc.Width = width;
    swap_desc.Height = height;
    swap_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap_desc.SampleDesc.Count = 1;
    swap_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_desc.BufferCount = 2;
    swap_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    ComPtr<IDXGISwapChain1> swap;
    check(factory->CreateSwapChainForHwnd(queue.Get(), window, &swap_desc, nullptr, nullptr, &swap),
        "CreateSwapChainForHwnd");
    check(swap.As(&swap_chain), "IDXGISwapChain3");
    check(factory->MakeWindowAssociation(window, DXGI_MWA_NO_ALT_ENTER), "MakeWindowAssociation");

    D3D12_DESCRIPTOR_HEAP_DESC heap_desc{};
    heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    heap_desc.NumDescriptors = 2;
    check(device->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&rtv_heap)), "Create RTV heap");
    rtv_stride = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    heap_desc.NumDescriptors = 1;
    check(device->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&dsv_heap)), "Create DSV heap");
    create_targets();
    check(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator)),
        "CreateCommandAllocator");
    check(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator.Get(), nullptr,
        IID_PPV_ARGS(&commands)), "CreateCommandList");
    check(commands->Close(), "Initial command list close");

    D3D12_ROOT_PARAMETER camera{};
    camera.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    camera.Constants = {0, 0, 20};
    camera.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    D3D12_ROOT_SIGNATURE_DESC root_desc{};
    root_desc.NumParameters = 1;
    root_desc.pParameters = &camera;
    root_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    ComPtr<ID3DBlob> signature, error;
    check(D3D12SerializeRootSignature(&root_desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error),
        "Serialize root signature");
    check(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(),
        IID_PPV_ARGS(&root_signature)), "CreateRootSignature");
    const D3D12_INPUT_ELEMENT_DESC input[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
    };
    D3D12_GRAPHICS_PIPELINE_STATE_DESC pso{};
    pso.pRootSignature = root_signature.Get();
    pso.VS = {scene_vs, sizeof(scene_vs)};
    pso.PS = {scene_ps, sizeof(scene_ps)};
    pso.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    pso.SampleMask = UINT_MAX;
    pso.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    pso.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    pso.RasterizerState.DepthClipEnable = TRUE;
    pso.DepthStencilState.DepthEnable = TRUE;
    pso.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    pso.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    pso.InputLayout = {input, 2};
    pso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pso.NumRenderTargets = 1;
    pso.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    pso.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    pso.SampleDesc.Count = 1;
    check(device->CreateGraphicsPipelineState(&pso, IID_PPV_ARGS(&pipeline)), "CreateGraphicsPipelineState");

    pso.DepthStencilState.DepthEnable = FALSE;
    pso.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    check(device->CreateGraphicsPipelineState(&pso, IID_PPV_ARGS(&overlay_pipeline)), "Create overlay pipeline");

    // This tiny immutable fixture can be read from an upload heap. A staging/default
    // heap path is not justified by its size; its owner still outlives every GPU use.
    D3D12_HEAP_PROPERTIES upload{};
    upload.Type = D3D12_HEAP_TYPE_UPLOAD;
    D3D12_RESOURCE_DESC buffer{};
    buffer.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    buffer.Width = vertices.size_bytes();
    buffer.Height = 1;
    buffer.DepthOrArraySize = 1;
    buffer.MipLevels = 1;
    buffer.SampleDesc.Count = 1;
    buffer.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    check(device->CreateCommittedResource(&upload, D3D12_HEAP_FLAG_NONE, &buffer,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertex_buffer)), "Create vertex buffer");
    void* mapped = nullptr;
    const D3D12_RANGE no_read{0, 0};
    check(vertex_buffer->Map(0, &no_read, &mapped), "Map vertex buffer");
    std::memcpy(mapped, vertices.data(), vertices.size_bytes());
    vertex_buffer->Unmap(0, nullptr);
    vertex_count = static_cast<UINT>(vertices.size());
    vertex_view = {vertex_buffer->GetGPUVirtualAddress(), static_cast<UINT>(vertices.size_bytes()), sizeof(Vertex)};
    if (dynamic_capacity) {
        buffer.Width=static_cast<UINT64>(dynamic_capacity)*sizeof(Vertex);
        check(device->CreateCommittedResource(&upload,D3D12_HEAP_FLAG_NONE,&buffer,
            D3D12_RESOURCE_STATE_GENERIC_READ,nullptr,IID_PPV_ARGS(&dynamic_buffer)),"Create dynamic triangle buffer");
        check(dynamic_buffer->Map(0,&no_read,&dynamic_mapped),"Map dynamic triangles");
        dynamic_view={dynamic_buffer->GetGPUVirtualAddress(),static_cast<UINT>(buffer.Width),sizeof(Vertex)};
        std::fprintf(stderr,"Dynamic triangle stream: capacity=%u vertices, persistent upload, one frame in flight.\n",dynamic_capacity);
    }
    check(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)), "CreateFence");
    fence_event = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    if (!fence_event) throw std::runtime_error("CreateEvent failed.");
    std::fprintf(stderr, "Static vertex buffer: %u vertices.\n", vertex_count);
}

void D3D12View::create_targets()
{
    auto handle = rtv_heap->GetCPUDescriptorHandleForHeapStart();
    for (UINT i = 0; i < 2; ++i) {
        check(swap_chain->GetBuffer(i, IID_PPV_ARGS(&back_buffers[i])), "Get back buffer");
        device->CreateRenderTargetView(back_buffers[i].Get(), nullptr, handle);
        handle.ptr += rtv_stride;
    }
    D3D12_HEAP_PROPERTIES heap{};
    heap.Type = D3D12_HEAP_TYPE_DEFAULT;
    D3D12_RESOURCE_DESC desc{};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Width = width;
    desc.Height = height;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = DXGI_FORMAT_D32_FLOAT;
    desc.SampleDesc.Count = 1;
    desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    D3D12_CLEAR_VALUE clear{};
    clear.Format = desc.Format;
    clear.DepthStencil.Depth = 1;
    check(device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &desc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE, &clear, IID_PPV_ARGS(&depth)), "Create depth buffer");
    device->CreateDepthStencilView(depth.Get(), nullptr, dsv_heap->GetCPUDescriptorHandleForHeapStart());
}

void D3D12View::wait_for_gpu()
{
    check(queue->Signal(fence.Get(), ++fence_value), "Queue signal");
    check(fence->SetEventOnCompletion(fence_value, fence_event), "Fence completion event");
    // Finite timeout makes device failure observable instead of hanging shutdown forever.
    if (WaitForSingleObject(fence_event, 10000) != WAIT_OBJECT_0)
        throw std::runtime_error("GPU fence wait failed or exceeded 10 seconds.");
    check(device->GetDeviceRemovedReason(), "Device removed");
}

void D3D12View::resize(UINT new_width, UINT new_height)
{
    if (new_width == width && new_height == height) return;
    if (new_width == 0 || new_height == 0) return;
    wait_for_gpu();
    for (auto& target : back_buffers) target.Reset();
    depth.Reset();
    check(swap_chain->ResizeBuffers(2, new_width, new_height, DXGI_FORMAT_R8G8B8A8_UNORM, 0), "ResizeBuffers");
    width = new_width;
    height = new_height;
    create_targets();
    std::fprintf(stderr, "Resized: %u x %u\n", width, height);
}

void D3D12View::draw(const DirectX::XMFLOAT4X4& view_projection, DirectX::XMFLOAT3 translation, std::span<const Vertex> dynamic_vertices)
{
    // One frame in flight keeps this first slice's ownership explicit. The previous
    // frame's fence has completed before reusing the allocator, depth or constants.
    if (dynamic_vertices.size()!=dynamic_capacity) throw std::runtime_error("Dynamic triangle count changed.");
    if (dynamic_capacity) {
        // Previous draw waits for its GPU fence; no GPU reader remains during this write.
        const auto begin=std::chrono::steady_clock::now();
        std::memcpy(dynamic_mapped,dynamic_vertices.data(),dynamic_vertices.size_bytes());
        dynamic_upload_us+=std::chrono::duration<double,std::micro>(std::chrono::steady_clock::now()-begin).count();
        ++dynamic_uploads;
    }
    check(allocator->Reset(), "Reset allocator");
    check(commands->Reset(allocator.Get(), pipeline.Get()), "Reset commands");
    commands->SetGraphicsRootSignature(root_signature.Get());
    commands->SetGraphicsRoot32BitConstants(0, 16, &view_projection, 0);
    const D3D12_VIEWPORT viewport{0, 0, static_cast<float>(width), static_cast<float>(height), 0, 1};
    const D3D12_RECT scissor{0, 0, static_cast<LONG>(width), static_cast<LONG>(height)};
    commands->RSSetViewports(1, &viewport);
    commands->RSSetScissorRects(1, &scissor);
    const UINT index = swap_chain->GetCurrentBackBufferIndex();
    auto barrier = transition(back_buffers[index].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    commands->ResourceBarrier(1, &barrier);
    auto rtv = rtv_heap->GetCPUDescriptorHandleForHeapStart();
    rtv.ptr += index * rtv_stride;
    const auto dsv = dsv_heap->GetCPUDescriptorHandleForHeapStart();
    const float sky[]{0.10f, 0.17f, 0.23f, 1};
    commands->OMSetRenderTargets(1, &rtv, FALSE, &dsv);
    commands->ClearRenderTargetView(rtv, sky, 0, nullptr);
    commands->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, 1, 0, 0, nullptr);
    commands->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commands->IASetVertexBuffers(0, 1, &vertex_view);
    const float zero_offset[4]{};
    const float offset[4]{translation.x, translation.y, translation.z, 0};
    commands->SetGraphicsRoot32BitConstants(0, 4, zero_offset, 16);
    commands->DrawInstanced(translated_vertex_start, 1, 0, 0);
    if (dynamic_capacity) {
        commands->IASetVertexBuffers(0,1,&dynamic_view);
        commands->DrawInstanced(dynamic_capacity,1,0,0);
        commands->IASetVertexBuffers(0,1,&vertex_view);
    }
    // Root constants are captured per draw; the immutable GPU buffer is never rewritten.
    commands->SetGraphicsRoot32BitConstants(0, 4, offset, 16);
    commands->DrawInstanced(overlay_vertex_start - translated_vertex_start, 1, translated_vertex_start, 0);
    // NDC triangles use the same shader/buffer; only overlay depth policy differs.
    DirectX::XMFLOAT4X4 identity;
    DirectX::XMStoreFloat4x4(&identity, DirectX::XMMatrixIdentity());
    commands->SetPipelineState(overlay_pipeline.Get());
    commands->SetGraphicsRoot32BitConstants(0, 16, &identity, 0);
    commands->SetGraphicsRoot32BitConstants(0, 4, zero_offset, 16);
    commands->DrawInstanced(vertex_count - overlay_vertex_start, 1, overlay_vertex_start, 0);
    barrier = transition(back_buffers[index].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    commands->ResourceBarrier(1, &barrier);
    check(commands->Close(), "Close commands");
    ID3D12CommandList* lists[]{commands.Get()};
    queue->ExecuteCommandLists(1, lists);
    check(swap_chain->Present(1, 0), "Present");
    wait_for_gpu();
    ++frame_count;
}

D3D12View::~D3D12View()
{
    if (fence && fence_event) {
        try { wait_for_gpu(); }
        catch (const std::exception& error) { std::fprintf(stderr, "Shutdown: %s\n", error.what()); }
    }
    commands.Reset();
    allocator.Reset();
    pipeline.Reset();
    overlay_pipeline.Reset();
    root_signature.Reset();
    if (dynamic_mapped) { dynamic_buffer->Unmap(0,nullptr); dynamic_mapped=nullptr; }
    dynamic_buffer.Reset();
    if (dynamic_uploads) std::fprintf(stderr,"Dynamic upload: samples=%llu mean_us=%.3f (CPU memcpy only)\n",
        dynamic_uploads,dynamic_upload_us/static_cast<double>(dynamic_uploads));
    vertex_buffer.Reset();
    depth.Reset();
    for (auto& target : back_buffers) target.Reset();
    rtv_heap.Reset();
    dsv_heap.Reset();
    swap_chain.Reset();
    queue.Reset();
    fence.Reset();
    if (fence_event) CloseHandle(fence_event);
#ifdef _DEBUG
    if (device) {
        ComPtr<ID3D12DebugDevice> debug_device;
        if (SUCCEEDED(device.As(&debug_device))) {
            std::fprintf(stderr, "Live-object report after releasing owned resources (device remains for reporting):\n");
            debug_device->ReportLiveDeviceObjects(static_cast<D3D12_RLDO_FLAGS>(
                D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL));
        }
    }
    std::fprintf(stderr, "D3D12 validation errors: %u\n", validation_errors.load());
    if (callback_registered) info_queue->UnregisterMessageCallback(callback_cookie);
    info_queue.Reset();
#endif
    device.Reset();
    std::fprintf(stderr, "Renderer shutdown; completed frames: %llu\n", frame_count);
}
}
