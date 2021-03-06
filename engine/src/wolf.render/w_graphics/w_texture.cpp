#include "w_render_pch.h"
#include "w_texture.h"
#include <w_convert.h>
#include <w_io.h>
#include "w_buffer.h"
#include "w_command_buffers.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace wolf
{
    namespace graphics
    {
        class w_texture_pimp
        {
        public:
            w_texture_pimp() :
                _name("w_texture"),
                _gDevice(nullptr),
                _width(0),
                _height(0),
                _image(0),
                _view(0),
                _sampler(0),
                _memory(0),
                _format(VK_FORMAT_R8G8B8A8_UNORM),
                _image_type(VkImageType::VK_IMAGE_TYPE_2D),
                _image_view_type(VkImageViewType::VK_IMAGE_VIEW_TYPE_2D)
            {
            }

            ~w_texture_pimp()
            {
                release();
            }

            HRESULT load(_In_ const std::shared_ptr<w_graphics_device>& pGDevice, _In_ const VkMemoryPropertyFlags pMemoryPropertyFlags)
            {
                this->_gDevice = pGDevice;
                this->_memory_property_flags = pMemoryPropertyFlags;
                return S_OK;
            }

            HRESULT initialize_texture_2D_from_file(_In_z_ std::wstring pPath, _In_ bool pIsAbsolutePath)
            {
                using namespace std;
                using namespace system::io;

                auto _path = (pIsAbsolutePath ? L"" : content_path) + pPath;
                auto _ext = get_file_extentionW(_path.c_str());

#if defined(__WIN32) || defined(__UWP)
                auto _c_str = _path.c_str();
#else
                auto _c_str = wolf::system::convert::wstring_to_string(_path).c_str();
#endif
                
                //lower it
                std::transform(_ext.begin(), _ext.end(), _ext.begin(), ::tolower);

                if (_ext == L".dds")
                {
                    std::vector<uint8_t> data;
                    int _file_status = -1;
                    
#if defined(__WIN32) || defined(__UWP)
                    system::io::read_binary_fileW(_c_str, data, _file_status);
#else
                    system::io::read_binary_file(_c_str, data, _file_status);
#endif
                    if (_file_status != 1)
                    {
                        std::wstring _msg = L"";
                        
                        if (_file_status == -1)
                        {
                            _msg = L"Could not find the texture file: ";
                        }
                        else
                        {
                            _msg = L"Texture file is corrupted: ";
                        }
                        V(S_FALSE, _msg + _path, this->_name, 3);

                        return S_FALSE;
                    }

                    logger.error(L"DDS format not supported on this platform for following file: " + _path);
                    return S_FALSE;
                }
                else if (_ext == L".jpg" || _ext == L".bmp" || _ext == L".png" || _ext == L".psd" || _ext == L".tga")
                {
                    if (S_FALSE == system::io::get_is_file(_c_str))
                    {
                        wstring msg = L"could not find the texture file: ";
                        V(S_FALSE, msg + _path, this->_name, 3);
                        
                        return S_FALSE;
                    }
                    else
                    {
                        int __width, __height, __comp;
                        uint8_t* _rgba = stbi_load(_c_str, &__width, &__height, &__comp, STBI_rgb_alpha);
                        if (_rgba)
                        {
                            this->_width = __width;
                            this->_height = __height;

                            if (this->_width == 0 || this->_height == 0)
                            {
                                wstring msg = L"Width or Height of texture file is zero: ";
                                V(S_FALSE, msg + _path, this->_name, 3);
                                return S_FALSE;
                            }

                            auto _hr = _create_image();
                            if (_hr == S_FALSE) return S_FALSE;

                            _hr = _allocate_memory();
                            if (_hr == S_FALSE) return S_FALSE;

                            //bind to memory
                            if (vkBindImageMemory(this->_gDevice->vk_device,
                                this->_image,
                                this->_memory,
                                0))
                            {
                                V(S_FALSE, "binding VkImage for graphics device: " + this->_gDevice->device_name +
                                    " ID: " + std::to_string(this->_gDevice->device_id), this->_name, 3, false, true);
                                return S_FALSE;
                            }

                            _hr = _create_image_view();
                            if (_hr == S_FALSE) return S_FALSE;

                            _hr = _create_sampler();
                            if (_hr == S_FALSE) return S_FALSE;

                            auto _size = this->_width * this->_height * 4;
                            std::vector<uint8_t> _data(_rgba, _rgba + _size);

                            _hr = copy_data_to_texture_2D(_data.data());
                            if (_hr == S_FALSE) return S_FALSE;

                            stbi_image_free(_rgba);

                            this->_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                            this->_image_info.sampler = this->_sampler;
                            this->_image_info.imageView = this->_view;

                        }
                        else
                        {
                            wstring msg = L"texture file is corrupted: ";
                            V(S_FALSE, msg + _path, this->_name, 3);
                            return S_FALSE;
                        }
                    }
                }
                else
                {
                    logger.error(L"Format not supported yet for following file: " + _path);
                    return S_FALSE;
                }

                return S_OK;
            }

            HRESULT initialize_texture_from_memory_rgba(_In_ uint8_t* pRGBAData, _In_ const UINT pWidth, _In_ const UINT pHeight)
            {
                this->_width = pWidth;
                this->_height = pHeight;

                auto _hr = _create_image();
                if (_hr == S_FALSE) return S_FALSE;

                _hr = _allocate_memory();
                if (_hr == S_FALSE) return S_FALSE;

                //bind to memory
                if (vkBindImageMemory(this->_gDevice->vk_device,
                    this->_image,
                    this->_memory,
                    0))
                {
                    V(S_FALSE, "binding VkImage for graphics device: " + this->_gDevice->device_name +
                        " ID: " + std::to_string(this->_gDevice->device_id), this->_name, 3, false, true);
                    return S_FALSE;
                }

                _hr = _create_image_view();
                if (_hr == S_FALSE) return S_FALSE;

                _hr = _create_sampler();
                if (_hr == S_FALSE) return S_FALSE;
                
                _hr = copy_data_to_texture_2D(pRGBAData);
                if (_hr == S_FALSE) return S_FALSE;
                
                this->_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                this->_image_info.sampler = this->_sampler;
                this->_image_info.imageView = this->_view;

                return S_OK;
            }

            HRESULT _create_image()
            {
                const VkImageCreateInfo _image_create_info =
                {
                    VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,                  // Type;
                    nullptr,                                              // Next
                    0,                                                    // Flags
                    this->_image_type,                                    // ImageType
                    this->_format,                                        // Format
                    {                                                     // Extent
                        this->_width,                                     // Width
                        this->_height,                                    // Height
                        1                                                 // Depth
                    },
                    1,                                                    // MipLevels
                    1,                                                    // ArrayLayers
                    VK_SAMPLE_COUNT_1_BIT,                                // Samples
                    VK_IMAGE_TILING_OPTIMAL,                              // Tiling
                    VK_IMAGE_USAGE_TRANSFER_DST_BIT |                     // Usage
                    VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_SHARING_MODE_EXCLUSIVE,                            // SharingMode
                    0,                                                    // QueueFamilyIndexCount
                    nullptr,                                              // QueueFamilyIndices
                    VK_IMAGE_LAYOUT_UNDEFINED                             // InitialLayout
                };
                
                //release the previous image
                if (this->_image)
                {
                    //first release it
                    vkDestroyImage(this->_gDevice->vk_device,
                                   this->_image,
                                   nullptr);
                }
                
                auto _hr = vkCreateImage( this->_gDevice->vk_device,
                                         &_image_create_info,
                                         nullptr,
                                         &this->_image );
                if (_hr)
                {
                    V(S_FALSE, "creating VkImage for graphics device: " + this->_gDevice->device_name +
                      " ID: " + std::to_string(this->_gDevice->device_id), this->_name, 3, false, true);
                    return S_FALSE;
                }
                
                return S_OK;
            }
            
            HRESULT _allocate_memory()
            {
                //release the old one
                if (this->_memory)
                {
                    vkFreeMemory(this->_gDevice->vk_device, this->_memory, nullptr);
                }
                
                VkMemoryRequirements _image_memory_requirements;
                vkGetImageMemoryRequirements(this->_gDevice->vk_device,
                                             this->_image,
                                             &_image_memory_requirements);
                
                VkPhysicalDeviceMemoryProperties _memory_properties;
                vkGetPhysicalDeviceMemoryProperties( this->_gDevice->vk_physical_device, &_memory_properties);
                
                bool _found = false;
                for( uint32_t i = 0; i < _memory_properties.memoryTypeCount; ++i )
                {
                    if( (_image_memory_requirements.memoryTypeBits & (1 << i)) &&
                       (_memory_properties.memoryTypes[i].propertyFlags & this->_memory_property_flags) )
                    {
                        
                        const VkMemoryAllocateInfo _memory_allocate_info =
                        {
                            VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,     // Type
                            nullptr,                                    // Next
                            _image_memory_requirements.size,            // AllocationSize
                            i                                           // MemoryTypeIndex
                        };
                        
                        if( vkAllocateMemory(this->_gDevice->vk_device,
                                             &_memory_allocate_info,
                                             nullptr,
                                             &this->_memory) == VK_SUCCESS )
                        {
                            _found = true;
                            break;
                        }
                    }
                }
                
                if(!_found)
                {
                    logger.error("Could not allocate memory for image");
                    return S_FALSE;
                }
                
                return S_OK;
            }
            
            HRESULT _create_image_view()
            {
                if(this->_view)
                {
                    vkDestroyImageView(this->_gDevice->vk_device, this->_view, nullptr );
                    this->_view = 0;
                }
                
                const VkImageViewCreateInfo _image_view_create_info =
                {
                    VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,             // Type
                    nullptr,                                              // Next
                    0,                                                    // Flags
                    this->_image,                                         // Image
                    this->_image_view_type,                               // ViewType
                    this->_format,                                        // Format
                    {                                                     // Components
                        VK_COMPONENT_SWIZZLE_IDENTITY,                        // VkComponentSwizzle         r
                        VK_COMPONENT_SWIZZLE_IDENTITY,                        // VkComponentSwizzle         g
                        VK_COMPONENT_SWIZZLE_IDENTITY,                        // VkComponentSwizzle         b
                        VK_COMPONENT_SWIZZLE_IDENTITY                         // VkComponentSwizzle         a
                    },
                    {                                                     // SubresourceRange
                        VK_IMAGE_ASPECT_COLOR_BIT,                            // VkImageAspectFlags         aspectMask
                        0,                                                    // uint32_t                   baseMipLevel
                        1,                                                    // uint32_t                   levelCount
                        0,                                                    // uint32_t                   baseArrayLayer
                        1                                                     // uint32_t                   layerCount
                    }
                };
                
                auto _hr = vkCreateImageView( this->_gDevice->vk_device,
                                             &_image_view_create_info,
                                             nullptr,
                                             &this->_view);
                if(_hr)
                {
                    V(S_FALSE, "creating image view", this->_name, false, true);
                    return S_FALSE;
                }
                
                return S_OK;
            }
            
            HRESULT _create_sampler()
            {
                const VkSamplerCreateInfo _sampler_create_info =
                {
                    VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,                // Type
                    nullptr,                                              // Next
                    0,                                                    // Flags
                    VK_FILTER_LINEAR,                                     // MagFilter
                    VK_FILTER_LINEAR,                                     // MinFilter
                    VK_SAMPLER_MIPMAP_MODE_LINEAR,                        // MipmapMode
                    VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,              // AddressModeU
                    VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,              // AddressModeV
                    VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,              // AddressModeW
                    0.0f,                                                 // MipLodBias
                    VK_TRUE,                                              // AnisotropyEnable
                    1.0f,                                                 // MaxAnisotropy
                    VK_FALSE,                                             // CompareEnable
                    VK_COMPARE_OP_NEVER,                                  // CompareOp
                    0.0f,                                                 // MinLod
                    0.0f,                                                 // MaxLod
                    VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,                   // BorderColor
                    VK_FALSE                                              // UnnormalizedCoordinates
                };
                
                auto _hr = vkCreateSampler( this->_gDevice->vk_device,
                                           &_sampler_create_info,
                                           nullptr,
                                           &this->_sampler);
                if (_hr)
                {
                    V(S_FALSE, "creating sampler", this->_name, false, true);
                    return S_FALSE;
                }
                
                return S_OK;
            }
            
            HRESULT copy_data_to_texture_2D(_In_ const uint8_t* pRGBA)
            {
                auto _data_size = this->_width * this->_height * 4;
                w_buffer _staging_buffer;
                auto _hResult = _staging_buffer.load_as_staging(this->_gDevice, _data_size);
                if(_hResult == S_FALSE)
                {
                    return _hResult;
                }
                
                _hResult = _staging_buffer.bind();
                if(_hResult == S_FALSE)
                {
                    return _hResult;
                }
                
                void* _staging_buffer_memory_pointer = nullptr;
                auto _hr = vkMapMemory(this->_gDevice->vk_device,
                                       _staging_buffer.get_memory(),
                                       0,
                                       _data_size,
                                       0,
                                       &_staging_buffer_memory_pointer );
                
                if(_hr)
                {
                    V(S_FALSE , "Could not map memory and upload texture data to a staging buffer on graphics device: " +
                        this->_gDevice->device_name + " ID: " + std::to_string(this->_gDevice->device_id),
                        this->_name,
                        3,
                        false,
                        true);
                    return S_FALSE;
                }
                
                memcpy( _staging_buffer_memory_pointer, pRGBA, (size_t)_data_size );
                
                VkMappedMemoryRange _flush_range =
                {
                    VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,              // Type
                    nullptr,                                            // Next
                    _staging_buffer.get_memory(),                       // Memory
                    0,                                                  // Offset
                    _data_size                                          // Size
                };
                
                vkFlushMappedMemoryRanges(this->_gDevice->vk_device,
                                          1,
                                          &_flush_range);
                vkUnmapMemory(this->_gDevice->vk_device,
                              _staging_buffer.get_memory());
                
                
                //create command buffer
                w_command_buffers _command_buffer;
                _command_buffer.load(this->_gDevice, 1);
                auto _cmd = _command_buffer.get_command_at(0);
                
                _command_buffer.begin(0, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
                {
                    VkImageSubresourceRange _image_subresource_range =
                    {
                        VK_IMAGE_ASPECT_COLOR_BIT,                      // AspectMask
                        0,                                              // BaseMipLevel
                        1,                                              // LevelCount
                        0,                                              // BaseArrayLayer
                        1                                               // LayerCount
                    };
                    
                    VkImageMemoryBarrier _image_memory_barrier_from_undefined_to_transfer_dst =
                    {
                        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,         // Type
                        nullptr,                                        // Next
                        0,                                              // SrcAccessMask
                        VK_ACCESS_TRANSFER_WRITE_BIT,                   // DstAccessMask
                        VK_IMAGE_LAYOUT_UNDEFINED,                      // OldLayout
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,           // NewLayout
                        VK_QUEUE_FAMILY_IGNORED,                        // SrcQueueFamilyIndex
                        VK_QUEUE_FAMILY_IGNORED,                        // DstQueueFamilyIndex
                        this->_image,                                   // image
                        _image_subresource_range                        // SubresourceRange
                    };
                    
                    vkCmdPipelineBarrier(_cmd,
                                         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                                         0,
                                         0,
                                         nullptr,
                                         0,
                                         nullptr,
                                         1,
                                         &_image_memory_barrier_from_undefined_to_transfer_dst);
                    
                    VkBufferImageCopy _buffer_image_copy_info =
                    {
                        0,                                    // BufferOffset
                        0,                                    // BufferRowLength
                        0,                                    // BufferImageHeight
                        {                                     // ImageSubresource
                            VK_IMAGE_ASPECT_COLOR_BIT,        // AspectMask
                            0,                                // MipLevel
                            0,                                // BaseArrayLayer
                            1                                 // LayerCount
                        },
                        {                                     // ImageOffset
                            0,                                // X
                            0,                                // Y
                            0                                 // Z
                        },
                        {                                     // ImageExtent
                            this->_width,                     // Width
                            this->_height,                    // Height
                            1                                 // Depth
                        }
                    };
                    
                    vkCmdCopyBufferToImage(_cmd,
                                          _staging_buffer.get_handle(),
                                           this->_image,
                                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                           1,
                                           &_buffer_image_copy_info);
                    
                    VkImageMemoryBarrier _image_memory_barrier_from_transfer_to_shader_read =
                    {
                        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,             // Type
                        nullptr,                                            // Next
                        VK_ACCESS_TRANSFER_WRITE_BIT,                       // SrcAccessMask
                        VK_ACCESS_SHADER_READ_BIT,                          // DstAccessMask
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,               // OldLayout
                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,           // NewLayout
                        VK_QUEUE_FAMILY_IGNORED,                            // SrcQueueFamilyIndex
                        VK_QUEUE_FAMILY_IGNORED,                            // DstQueueFamilyIndex
                        this->_image,                                       // Image
                        _image_subresource_range                            // SubresourceRange
                    };
                    vkCmdPipelineBarrier(_cmd,
                                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                         0,
                                         0,
                                         nullptr,
                                         0,
                                         nullptr,
                                         1,
                                         &_image_memory_barrier_from_transfer_to_shader_read);
                
                }
                _command_buffer.end(0);
                    
                // Submit command buffer and copy data from staging buffer to a vertex buffer
                VkSubmitInfo _submit_info =
                {
                        VK_STRUCTURE_TYPE_SUBMIT_INFO,        // Type
                        nullptr,                              // Next
                        0,                                    // WaitSemaphoreCount
                        nullptr,                              // WaitSemaphores
                        nullptr,                              // WaitDstStageMask;
                        1,                                    // CommandBufferCount
                        &_cmd,                                // CommandBuffers
                        0,                                    // SignalSemaphoreCount
                        nullptr                               // SignalSemaphores
                };
                
                _hr = vkQueueSubmit( this->_gDevice->vk_graphics_queue.queue, 1, &_submit_info, VK_NULL_HANDLE );
                if (_hr)
                {
                    V(S_FALSE , "Could submit map memory and upload texture data to a staging buffer on graphics device: " +
                      this->_gDevice->device_name + " ID: " + std::to_string(this->_gDevice->device_id),
                      this->_name,
                      3,
                      false,
                      true);
                    
                    _command_buffer.release();
                    return S_FALSE;
                }
                vkDeviceWaitIdle(this->_gDevice->vk_device);
                
                //release command buffer
                _command_buffer.release();
                
                return S_OK;
            }
            
            const VkDescriptorImageInfo get_descriptor_info() const
            {
                return this->_image_info;
            }
            
            ULONG release()
            {
                //release sampler
                if(this->_sampler)
                {
                    vkDestroySampler(this->_gDevice->vk_device,
                                     this->_sampler,
                                     nullptr);
                    this->_sampler = 0;
                }
                
                //release view
                if(this->_view)
                {
                    vkDestroyImageView(this->_gDevice->vk_device, this->_view, nullptr );
                    this->_view = 0;
                }
                
                //release image
                if (this->_image)
                {
                    vkDestroyImage(this->_gDevice->vk_device,
                                   this->_image,
                                   nullptr);
                    this->_image= 0;
                }
                
                //release memory
                if( this->_memory)
                {
                    vkFreeMemory(this->_gDevice->vk_device,
                                 this->_memory,
                                 nullptr);
                    this->_memory= 0;
                }
                
                this->_gDevice = nullptr;
                
                return 1;
            }
            
            
#pragma region Getters
            
            const UINT get_width() const
            {
                return this->_width;
            }
            
            const UINT get_height() const
            {
                return this->_height;
            }
            
            VkSampler get_sampler() const
            {
                return this->_sampler;
            }
           
            VkImage get_image() const
            {
                return this->_image;
            }
            
            VkImageView get_image_view() const
            {
                return this->_view;
            }
            
            VkImageType get_image_type() const
            {
                return this->_image_type;
            }
            
            VkImageViewType get_image_view_type() const
            {
                return this->_image_view_type;
            }
            
            VkFormat get_format() const
            {
                return this->_format;
            }
            
#pragma endregion
                   
        private:
            
            std::string                                     _name;
            std::shared_ptr<w_graphics_device>              _gDevice;
            uint32_t                                        _width;
            uint32_t                                        _height;
            VkMemoryPropertyFlags                           _memory_property_flags;
            VkImage                                         _image;
            VkImageView                                     _view;
            VkSampler                                       _sampler;
            VkDeviceMemory                                  _memory;
            VkFormat                                        _format;
            VkImageType                                     _image_type;
            VkImageViewType                                 _image_view_type;
            VkDescriptorImageInfo                           _image_info;
        };
    }
}

using namespace wolf::graphics;

std::map<std::wstring, w_texture*> w_texture::_shared;
w_texture* w_texture::default_texture = nullptr;

w_texture::w_texture() : 
    _pimp(new w_texture_pimp())
{
	_super::set_class_name("w_texture_2d");
}

w_texture::~w_texture()
{
	release();
}

HRESULT w_texture::load(_In_ const std::shared_ptr<w_graphics_device>& pGDevice, _In_ const VkMemoryPropertyFlags pMemoryPropertyFlags)
{
    if(!this->_pimp) return S_FALSE;
    return this->_pimp->load(pGDevice, pMemoryPropertyFlags);
}

HRESULT w_texture::initialize_texture_2D_from_file(_In_z_ std::wstring pPath, _In_ bool pIsAbsolutePath)
{
    if(!this->_pimp) return S_FALSE;
    return this->_pimp->initialize_texture_2D_from_file(pPath, pIsAbsolutePath);
}

HRESULT w_texture::initialize_texture_from_memory_rgba(_In_ uint8_t* pRGBAData, _In_ const UINT pWidth, _In_ const UINT pHeight)
{
    if (!this->_pimp) return S_FALSE;
    return this->_pimp->initialize_texture_from_memory_rgba(pRGBAData, pWidth, pHeight);
}

HRESULT w_texture::initialize_texture_from_memory_rgb(_In_ uint8_t* pRGBData, _In_ const UINT pWidth, _In_ const UINT pHeight)
{
    if (!this->_pimp || pWidth == 0 || pHeight == 0) return S_FALSE;

    auto _rgba = (uint8_t*)malloc(pWidth * pHeight * 4);
    if (!_rgba) return S_FALSE;
    
    size_t _count = pWidth * pHeight;
    for (int i = _count; --i; _rgba += 4, pRGBData += 3) 
    {
        *(uint32_t*)(void*)_rgba = *(const uint32_t*)(const void*)pRGBData;
    }
    for (int j = 0; j < 3; ++j)
    {
        _rgba[j] = pRGBData[j];
    }

    return this->_pimp->initialize_texture_from_memory_rgba(_rgba, pWidth, pHeight);
}

HRESULT w_texture::initialize_texture_from_memory_all_channels_same(_In_ uint8_t pData, _In_ const UINT pWidth, _In_ const UINT pHeight)
{
    if (!this->_pimp) return S_FALSE;

    auto _rgba = (uint8_t*)malloc(pWidth * pHeight * 4);
    if (!_rgba) return S_FALSE;

    std::memset(_rgba, pData, pWidth * pHeight);
    return this->_pimp->initialize_texture_from_memory_rgba(_rgba, pWidth, pHeight);
}

HRESULT w_texture::load_to_shared_textures(_In_ const std::shared_ptr<w_graphics_device>& pGDevice,
    _In_z_ std::wstring pPath,
    _Inout_ w_texture** pPointerToTexture)
{
    //check if already exists
    auto _iter = _shared.find(pPath);
    if (_iter != _shared.end())
    {
        *pPointerToTexture = _shared[pPath];
        return S_OK;
    }

    auto _texture = new (std::nothrow) w_texture();
    if (!_texture)
    {
        logger.error(L"Could not perform allocation for shared texture: " + pPath);
        return S_FALSE;
    }

    auto _hr = _texture->load(pGDevice);
    if (_hr == S_FALSE)
    {
        SAFE_RELEASE(_texture);
        return S_FALSE;
    }
    _hr = _texture->initialize_texture_2D_from_file(pPath, true);
    if (_hr == S_FALSE)
    {
        SAFE_RELEASE(_texture);
        return S_FALSE;
    }

    _shared[pPath] = _texture;
    *pPointerToTexture = _texture;

    return S_OK;
}

void w_texture::write_bitmap_to_file(
    _In_z_ const char* pFilename,
    _In_ const uint8_t* pData,
    _In_ const int& pWidth, const int& pHeight)
{
    short header[] = { 0x4D42, 0, 0, 0, 0, 26, 0, 12, 0, (short)pWidth, (short)pHeight, 1, 24 };
    FILE *f;
    
#ifdef __WIN32
    if (!fopen_s(&f, pFilename, "wb"))
#else
    f = fopen(pFilename, "wb");
    if (!f)
#endif
    {
        fwrite(header, 1, sizeof(header), f);
        fwrite(pData, 1, pWidth * pHeight * 3, f);
        fclose(f);
    }
}

ULONG w_texture::release()
{
	if (_super::get_is_released()) return 0;
    
    SAFE_RELEASE(this->_pimp);
    
	return _super::release();
}

ULONG w_texture::release_shared_textures()
{
    if (!_shared.size()) return 0;

    default_texture = nullptr;
    for (auto _pair : _shared)
    {
        SAFE_RELEASE(_pair.second);
    }
    _shared.clear();

    return 1;
}

#pragma region Getters

const UINT w_texture::get_width() const
{
    if(!this->_pimp) return 0;
    return this->_pimp->get_width();
}

const UINT w_texture::get_height() const
{
    if(!this->_pimp) return 0;
    return this->_pimp->get_height();
}

//get sampler of image
VkSampler w_texture::get_sampler() const
{
    if(!this->_pimp) return 0;
    return this->_pimp->get_sampler();
}

//get image handle
VkImage w_texture::get_image() const
{
    if(!this->_pimp) return 0;
    return this->_pimp->get_image();
}

//get image view resource
VkImageView w_texture::get_image_view() const
{
    if(!this->_pimp) return 0;
    return this->_pimp->get_image_view();
}

//get image type
VkImageType w_texture::get_image_type() const
{
    if(!this->_pimp) return VkImageType::VK_IMAGE_TYPE_1D;
    return this->_pimp->get_image_type();
}

//get image view type
VkImageViewType w_texture::get_image_view_type() const
{
    if(!this->_pimp) return VkImageViewType::VK_IMAGE_VIEW_TYPE_1D;
    return this->_pimp->get_image_view_type();
}

//get image format
VkFormat w_texture::get_format() const
{
    if(!this->_pimp) return VkFormat::VK_FORMAT_UNDEFINED;
    return this->_pimp->get_format();
}

const VkDescriptorImageInfo w_texture::get_descriptor_info() const
{
    if(!this->_pimp) return VkDescriptorImageInfo();
    return this->_pimp->get_descriptor_info();
}

#pragma endregion

