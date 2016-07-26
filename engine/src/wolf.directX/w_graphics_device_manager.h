/*
	Project			 : Wolf Engine. Copyright(c) Pooya Eimandar (http://PooyaEimandar.com) . All rights reserved.
	Source			 : Please direct any bug to https://github.com/PooyaEimandar/Wolf.Engine/issues
	Website			 : http://WolfSource.io
	Name			 : w_graphics_device_manager.h
	Description		 : The manager for graphics devices
	Comment          :
*/

#ifndef __W_GRAPHICS_DEVICE_MANAGER_H__
#define __W_GRAPHICS_DEVICE_MANAGER_H__

#include "w_directX_dll.h"
#include <wrl\client.h>
#include <algorithm>
#include <memory>
#include <vector>
#include <map>
#include <Wincodec.h> // for IWICImagingFactory2

#ifdef __DX12__

#include <d3d12.h>
#include <d2d1_3.h>
#include <dwrite_3.h>
#include <dxgi1_4.h>

#elif defined(__DX11_X__)

#include <d3d11_2.h>
#include <d2d1_1.h>
#include <dwrite_1.h>
#include <dxgi1_3.h>
#include <dxgi1_2.h>

#endif

#include <DirectXMath.h>

//for directX graphics diagnostic debugging and capturing frame
#if defined(_DEBUG) && defined(__DX_DIAGNOSTIC__)

#include <DXGItype.h>
#include <DXProgrammableCapture.h>

#endif

//C++AMP
#include <amp.h>
#include <amp_graphics.h>
#include <amp_math.h>

//Wolf
#include <w_object.h>
#include <w_window.h>

#define DEFAULT_DPI 96

namespace wolf
{
	namespace graphics
	{
		enum GPGPU_TYPE { CPP_AMP, OPENCL, NONE };
		enum CPP_AMP_DEVICE_TYPE { DEFAULT, CPU, GPU, GPU_REF, GPU_WARP};

		//Output window which handles all direct 3d resources for output renderer
		struct w_output_window
		{
		public:
			//Add refrence
			ULONG AddRef() { return 1; }
			//Release resources
			ULONG release()
			{
				if (this->isReleased) return 0;
				this->isReleased = true;

				this->hWnd = NULL;

				COMPTR_RELEASE(this->backBuffer);
				COMPTR_RELEASE(this->depthStencilView);
				COMPTR_RELEASE(this->renderTargetView);
				COMPTR_RELEASE(this->swapChain);

				return 1;
			}

			UINT																index;
			bool																isReleased;
			HWND																hWnd;
			UINT																width;
			UINT																height;
			DWORD																pdwCookie;
			float																aspectRatio;
			CD3D11_VIEWPORT														viewPort;
			Microsoft::WRL::ComPtr<IDXGISwapChain1>								swapChain;
			Microsoft::WRL::ComPtr<ID3D11Texture2D>								backBuffer;
			Microsoft::WRL::ComPtr<ID3D11RenderTargetView>						renderTargetView;
			Microsoft::WRL::ComPtr<ID3D11DepthStencilView>						depthStencilView;
		};

		//Contains direct3D device and context which performs primitive-based rendering
		class w_graphics_device
		{
			friend class w_graphics_device_manager;
		public:
			//Get the first and the primary window which was created with this device
			w_output_window* MainWindow()
			{
				if (this->output_windows.size() > 0)
				{
					auto window = this->output_windows.at(0);
					return &window;
				}
				return nullptr;
			}

			//Release all resources
			ULONG release()
			{
				if (this->_is_released) return 0;
				this->_is_released = true;

				auto _size = this->command_queue.size();
				if (_size > 0)
				{
					for (size_t i = 0; i < _size; ++i)
					{
						auto _cq = this->command_queue.at(i);
						if (_cq)
						{
							_cq->Release();
							_cq = nullptr;
						}
					}
					this->command_queue.clear();
				}

				COMPTR_RELEASE(this->d2D_multithread);
				COMPTR_RELEASE(this->image_factory);
				COMPTR_RELEASE(this->factory_2D);
				COMPTR_RELEASE(this->write_factory);
				COMPTR_RELEASE(this->context_2D);
				COMPTR_RELEASE(this->device_2D);

				for (size_t i = 0; i < this->output_windows.size(); ++i)
				{
					this->output_windows.at(i).release();
				}
				this->output_windows.clear();
				this->dxgi_outputs_desc.clear();

				COMPTR_RELEASE(this->context);
				COMPTR_RELEASE(this->device);

				COMPTR_RELEASE(this->adaptor);

				return 1;
			}

			bool																direct2D_multithreaded;
			D3D_FEATURE_LEVEL													feature_level;

			Microsoft::WRL::ComPtr<IDXGIAdapter>								adaptor;
			std::vector<DXGI_OUTPUT_DESC>										dxgi_outputs_desc;
			std::vector<w_output_window>										output_windows;

#if defined(__DX12__)
			Microsoft::WRL::ComPtr<ID3D12Device>								device;
#elif defined(__DX11_X__)
			Microsoft::WRL::ComPtr<ID3D11Device1>								device;
			Microsoft::WRL::ComPtr<ID3D11DeviceContext1>						context;
#endif

			Microsoft::WRL::ComPtr<ID2D1Device>									device_2D;
			Microsoft::WRL::ComPtr<ID2D1DeviceContext>							context_2D;
			Microsoft::WRL::ComPtr<ID2D1Factory1>								factory_2D;
			Microsoft::WRL::ComPtr<ID2D1Bitmap1>								target_2D;
			Microsoft::WRL::ComPtr<ID2D1Multithread>							d2D_multithread;

			Microsoft::WRL::ComPtr<IDWriteFactory>								write_factory;
			Microsoft::WRL::ComPtr<IWICImagingFactory2>							image_factory;

			std::vector<ID3D11CommandList*>										command_queue;

#ifdef _DEBUG
			Microsoft::WRL::ComPtr<ID3D11Debug>									d3d_debug_layer;
#endif

			DX_EXP void create_blend_state(
				BOOL pBlendEnable,
				D3D11_BLEND pSrcBlend, D3D11_BLEND pDestBlend,
				D3D11_BLEND pSrcBlendAlpha, D3D11_BLEND pDestBlendAlpha,
				D3D11_BLEND_OP pBlendOp, D3D11_BLEND_OP pBlendOpAlpha,
				UINT8 pRenderTargetWriteMask,
				_Out_ ID3D11BlendState1** pBlendstate);

			DX_EXP void create_depth_stencil_state(bool pEnable, bool pWriteEnable, _Out_ ID3D11DepthStencilState** pDepthStencilState);
			DX_EXP void create_rasterizer_state(D3D11_CULL_MODE pCullMode, D3D11_FILL_MODE pFillMode, _Out_ ID3D11RasterizerState** pRasterizerState);
			DX_EXP void create_sampler_state(D3D11_FILTER pFilter, D3D11_TEXTURE_ADDRESS_MODE pAddressMode, _Out_ ID3D11SamplerState** pSamplerState);
			DX_EXP void enable_alpha_blend(std::initializer_list<float> pBlendFactor = { 0.0f, 0.0f, 0.0f, 0.0f }, UINT pSampleMask = 0xFFFFFFFF);

		private:
			bool _is_released;

			Microsoft::WRL::ComPtr<ID3D11BlendState1>							_opaque;
			Microsoft::WRL::ComPtr<ID3D11BlendState1>							_alpha_blend;
			Microsoft::WRL::ComPtr<ID3D11BlendState1>							_additive;
			Microsoft::WRL::ComPtr<ID3D11BlendState1>							_non_pre_multiplied;
		};

		//Handles the configuration and management of the graphics device.
		class w_graphics_device_manager : public system::w_object
		{
		public:
			DX_EXP w_graphics_device_manager(bool pUse_Wrap_Mode = false);
			DX_EXP virtual ~w_graphics_device_manager();

			//Initialize graphics devices
			DX_EXP virtual void initialize();
			//Initialize the output windows
			DX_EXP virtual void initialize_output_windows(std::map<int, std::vector<W_WindowInfo>> pOutputWindowsInfo);
			//Called when corresponding window resized
			DX_EXP virtual void on_window_resized(UINT pIndex);
			//Called when any graphics device lost
			DX_EXP virtual void on_device_lost();
			//Begin render on all graphics devices
			DX_EXP virtual void begin_render();
			//End render on all graphics devices
			DX_EXP virtual void end_render();
			//Release all resources
			DX_EXP ULONG release() override;

			DX_EXP static std::vector<Microsoft::WRL::ComPtr<ID3D11SamplerState>>	samplers;
			DX_EXP static std::unique_ptr<concurrency::accelerator_view>  cAmpAcc;

#if defined(_DEBUG) && defined(__DX_DIAGNOSTIC__)
			DX_EXP static Microsoft::WRL::ComPtr<IDXGraphicsAnalysis> graphics_diagnostic;
#endif

#pragma region Getters

			//Get the main graphics device, this is first and the primary device.
			std::shared_ptr<w_graphics_device> get_graphics_device() const		{ return this->_graphics_devices.size() > 0 ? this->_graphics_devices.at(0) : nullptr; }
			//Returns number of available graphics devices
			const ULONG get_number_of_graphics_devices() const					{ return static_cast<ULONG>(this->_graphics_devices.size()); }
			//Get deafult window HWND
			const HWND get_window_hWnd() const									{ return this->_windowsInfo.size() == 0 || this->_windowsInfo.at(0).size() == 0 ? NULL : this->_windowsInfo.at(0).at(0).hWnd; }
			//Get deafult window width
			const UINT get_window_width() const									{ return this->_windowsInfo.size() == 0 || this->_windowsInfo.at(0).size() == 0 ? 0 : this->_windowsInfo.at(0).at(0).width; }
			//Get deafult window height
			const UINT get_window_height() const								{ return this->_windowsInfo.size() == 0 || this->_windowsInfo.at(0).size() == 0 ? 0 : this->_windowsInfo.at(0).at(0).height; }
			//Get DPI
			const DirectX::XMFLOAT2 get_dpi() const;
			//Get pixels to inches
			const DirectX::XMFLOAT2 get_pixels_to_inches(float pX, float pY) const;

#pragma endregion

#pragma region Setters

			void Set_Cpp_AMP_Device_Type(CPP_AMP_DEVICE_TYPE pValue)			{ this->_cpp_AMP_Device_Type = pValue; }
		
#pragma endregion

		protected:
			float																_backColor[4];
			std::vector<std::shared_ptr<w_graphics_device>>						_graphics_devices;

		private:
			typedef  system::w_object											_super;

			void	_enumerate_devices();
			void	_create_device();
			HRESULT _create_swapChain_for_window(std::shared_ptr<w_graphics_device> pGDevice, w_output_window& pOutputWindow);

			bool																_use_warp_mode;
			std::map<int, std::vector<W_WindowInfo>>							_windowsInfo;
			CPP_AMP_DEVICE_TYPE													_cpp_AMP_Device_Type;

#if defined (__DX12__)
			Microsoft::WRL::ComPtr<IDXGIFactory4>								_dxgi_factory;
#elif defined (__DX11_X__)
			Microsoft::WRL::ComPtr<IDXGIFactory2>								_dxgi_factory;
#endif
		};
	}
}

#endif