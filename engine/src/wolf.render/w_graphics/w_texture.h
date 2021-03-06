/*
	Project			 : Wolf Engine. Copyright(c) Pooya Eimandar (http://PooyaEimandar.com) . All rights reserved.
	Source			 : Please direct any bug to https://github.com/PooyaEimandar/Wolf.Engine/issues
	Website			 : http://WolfSource.io
	Name			 : w_texture_2d.h
	Description		 : create and handle 2D texture resource
	Comment          : Wolf used encoding texture with two channel based on http://devkk.net/index.php?tag=articles&id=24
*/

#ifndef __W_TEXTURE_2D_H__
#define __W_TEXTURE_2D_H__

#include "w_graphics_device_manager.h"
#include <w_logger.h>
#include <w_io.h>

namespace wolf
{
	namespace graphics
	{
        class w_texture_pimp;
		class w_texture : public system::w_object
		{
		public:
			W_EXP w_texture();
			W_EXP virtual ~w_texture();

            W_EXP HRESULT load(_In_ const std::shared_ptr<w_graphics_device>& pGDevice,
                               _In_ const VkMemoryPropertyFlags pMemoryPropertyFlags =
                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            
			//Load texture2D from file
			W_EXP HRESULT initialize_texture_2D_from_file(_In_ std::wstring pPath, _In_ bool pIsAbsolutePath = false);
            //Load texture2D from memory in format of RGBA
            W_EXP HRESULT initialize_texture_from_memory_rgba(_In_ uint8_t* pRGBAData, _In_ const UINT pWidth, _In_ const UINT pHeight);
            //Load texture2D from memory in format of RGB
            W_EXP HRESULT initialize_texture_from_memory_rgb(_In_ uint8_t* pRGBAData, _In_ const UINT pWidth, _In_ const UINT pHeight);
            //Load texture2D from memory from single channel
            W_EXP HRESULT initialize_texture_from_memory_all_channels_same(_In_ uint8_t pData, _In_ const UINT pWidth, _In_ const UINT pHeight);

            //release all resources
            W_EXP virtual ULONG release() override;

            //load texture and store it into the shared
            W_EXP static HRESULT load_to_shared_textures(_In_ const std::shared_ptr<w_graphics_device>& pGDevice, 
                                                         _In_z_ std::wstring pPath,
                                                        _Inout_ w_texture** pPointerToTexture);

            W_EXP static void write_bitmap_to_file(
                _In_z_ const char* pFilename,
                _In_ const uint8_t* pData,
                _In_ const int& pWidth, const int& pHeight);

            //release all shared textures
            W_EXP ULONG static release_shared_textures();
#pragma region Getters

            //get width of image
            W_EXP const UINT get_width() const;
            //get height of image
            W_EXP const UINT get_height() const;
            //get sampler of image
            W_EXP VkSampler get_sampler() const;
            //get image handle
            W_EXP VkImage get_image() const;
            //get image view resource
            W_EXP VkImageView get_image_view() const;
            //get image type
            W_EXP VkImageType get_image_type() const;
            //get image view type
            W_EXP VkImageViewType get_image_view_type() const;
            //get image format
            W_EXP VkFormat get_format() const;
            //get write descriptor image info
            W_EXP const VkDescriptorImageInfo get_descriptor_info() const;
            
#pragma endregion


            W_EXP static w_texture*                               default_texture;

		private:
			typedef system::w_object						_super;
            w_texture_pimp*                                 _pimp;

            static std::map<std::wstring, w_texture*>       _shared;
        };
    }
}

#endif
