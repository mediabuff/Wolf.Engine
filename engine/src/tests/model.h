/*
	Project			 : Wolf Engine. Copyright(c) Pooya Eimandar (http://PooyaEimandar.com) . All rights reserved.
	Source			 : Please direct any bug to https://github.com/WolfSource/Wolf.Engine/issues
	Website			 : http://WolfSource.io
	Name			 : model.h
	Description		 : The model
	Comment          :
*/

#ifndef __MODEL_H__
#define __MODEL_H__

#include <w_graphics_device_manager.h>
#include <w_cpipeline_model.h>
#include <w_graphics/w_mesh.h>
#include <w_graphics/w_uniform.h>
#include <w_graphics/w_shader.h>
#include <w_graphics/w_pipeline.h>
#include <w_graphics/w_render_pass.h>
#include <cameras/w_first_person_camera.h>
#include "masked_occlusion_culling/MaskedOcclusionCulling.h"

#define MAX_LOD_LEVEL 2

struct clipspace_vertex { float x, y, z, w; };

struct search_item_struct
{
    std::string                                 name;
    wolf::content_pipeline::w_bounding_sphere   bounding_sphere;
};

class model : public wolf::system::w_object
{
public:
    W_EXP model();
    virtual ~model();

    HRESULT load(
        _In_ const std::shared_ptr<wolf::graphics::w_graphics_device>& pGDevice, 
        _In_ wolf::content_pipeline::w_cpipeline_model* pCPModel,
        _In_ wolf::graphics::w_render_pass& pRenderPass);

    void pre_update(
        _In_    wolf::content_pipeline::w_first_person_camera pCamera,
        _Inout_ MaskedOcclusionCulling** sMOC);

    bool post_update(_Inout_ MaskedOcclusionCulling* sMOC);

    void indirect_draw(_In_ const VkCommandBuffer& pCommandBuffer);

    HRESULT submit_compute_shader(_In_ const wolf::content_pipeline::w_first_person_camera* pCamera);

    //Release will be called once per game and is the place to unload assets and release all resources
    ULONG release() override;

    void search_for_name(
        _In_z_ const std::string& pToBeFind, 
        _Inout_ std::vector<search_item_struct>& pResults);

#pragma region Getters

    const char*     get_full_name() const;
    glm::vec3       get_position() const;
    VkSemaphore     get_compute_semaphore() const;

#pragma endregion

#pragma region Setters

    void set_opacity(const float& pValue); 
    void set_color(const glm::vec4& pValue);

#pragma endregion

private:
    typedef wolf::system::w_object                          _super;

    void get_searchable_name(_In_z_ const std::string& pName);

    void _store_to_batch(
        _In_ const std::vector<wolf::content_pipeline::w_cpipeline_model::w_mesh*>& pModelMeshes,
        _Inout_ std::vector<float>& pVertices,
        _Inout_ std::vector<uint32_t>& pIndices,
        _Inout_ uint32_t& pBaseVertex);

    void _add_data_for_masked_occlusion_culling(_In_ const wolf::content_pipeline::w_bounding_box& pBoundingBox);

    HRESULT _load_shader();
    HRESULT _load_buffers();
    HRESULT _load_pipelines(_In_ wolf::graphics::w_render_pass& pRenderPass);
    HRESULT _build_compute_command_buffers();

    std::shared_ptr<wolf::graphics::w_graphics_device>      _gDevice;
    std::atomic<bool>                                       _loaded;
    std::string                                             _full_name;

    //unique name of factory for searching
    std::vector<std::string>                                _search_names;

    wolf::content_pipeline::w_transform_info               _transform;
    std::vector<wolf::content_pipeline::w_instance_info>   _instances_transforms;

    wolf::graphics::w_mesh*                                 _mesh;
    wolf::graphics::w_shader*                               _shader;

    //World view Projections o f root and all instances
    glm::mat4                                               _view_projection;
    std::vector<float>                                      _visibilities;

    //masked occlusion culling
    struct moc_data
    {
        //vertex for masked occlusion culling
        std::vector<clipspace_vertex>                           vertices;
        std::vector<uint32_t>                                   indices;
        int                                                     num_of_tris_for_moc;
        glm::vec3                                               position;
        glm::vec3                                               rotation;

        void release()
        {
            this->vertices.clear();
            this->indices.clear();
        }
    };
    std::vector<moc_data>                                       _mocs;
    wolf::content_pipeline::w_bounding_sphere                   _root_bounding_sphere;

    struct lod
    {
        uint32_t    first_index;
        uint32_t    index_count;
        float       distance;
        float       _padding;
    };
    std::vector<lod>                                        _lod_levels;

    wolf::graphics::w_vertex_binding_attributes             _vertex_binding_attributes;

    struct vertex_instance_data
    {
        glm::vec3   pos;
        glm::vec3   rot;
    };

    struct vertex_unifrom
    {
        glm::mat4 projection_view;
    };

#pragma pack(push,1)
    struct compute_instance_data
    {
        glm::vec4   pos;
    };
#pragma pack(pop)

#pragma region compute uniforms

#pragma pack(push,1)
    struct compute_unifrom_x1
    {
        glm::vec4           camera_pos;
        glm::vec4	        is_visible;
    };
#pragma pack(pop)

#pragma pack(push,1)
    struct compute_unifrom_x2
    {
        glm::vec4           camera_pos;
        glm::vec4	        is_visible;
    };
#pragma pack(pop)

#pragma pack(push,1)
    struct compute_unifrom_x4
    {
        glm::vec4           camera_pos;
        glm::vec4	        is_visible;
    };
#pragma pack(pop)

#pragma pack(push,1)
    struct compute_unifrom_x8
    {
        glm::vec4           camera_pos;
        glm::vec4	        is_visible[2];
    };
#pragma pack(pop)

#pragma pack(push,1)
    struct compute_unifrom_x16
    {
        glm::vec4           camera_pos;
        glm::vec4	        is_visible[4];
    };
#pragma pack(pop)

#pragma pack(push,1)
    struct compute_unifrom_x32
    {
        glm::vec4           camera_pos;
        glm::vec4	        is_visible[8];
    };
#pragma pack(pop)

#pragma pack(push,1)
    struct compute_unifrom_x64
    {
        glm::vec4           camera_pos;
        glm::vec4	        is_visible[16];
    };
#pragma pack(pop)

#pragma pack(push,1)
    struct compute_unifrom_x128
    {
        glm::vec4           camera_pos;
        glm::vec4	        is_visible[32];
    };
#pragma pack(pop)

#pragma pack(push,1)
    struct compute_unifrom_x256
    {
        glm::vec4           camera_pos;
        glm::vec4	        is_visible[64];
    };
#pragma pack(pop)

#pragma endregion

#pragma pack(push,1)
    struct color_unifrom
    {
        glm::vec4 color = glm::vec4(1, 1, 1, 1);
    };
#pragma pack(pop)

    struct vertex_stage
    {
        wolf::graphics::w_uniform<vertex_unifrom>               unifrom;
        wolf::graphics::w_buffer                                instance_buffer;
        wolf::graphics::w_pipeline                              pipeline;

        void release()
        {
            this->unifrom.release();
            this->instance_buffer.release();
            this->pipeline.release();
        }
    } vs;

    struct fragment_stage
    {
        wolf::graphics::w_texture*                              texture = nullptr;
        wolf::graphics::w_uniform<color_unifrom>                unifrom;

        void release()
        {
            this->unifrom.release();
            //texture will be release at the end of program
            //SAFE_RELEASE(this->texture);
        }
    } fs;

    struct compute_stage
    {
        uint32_t                                                batch_local_size = 1;
        
        wolf::graphics::w_uniform<compute_unifrom_x1>*          unifrom_x1;
        wolf::graphics::w_uniform<compute_unifrom_x2>*          unifrom_x2;
        wolf::graphics::w_uniform<compute_unifrom_x4>*          unifrom_x4;
        wolf::graphics::w_uniform<compute_unifrom_x8>*          unifrom_x8;
        wolf::graphics::w_uniform<compute_unifrom_x16>*         unifrom_x16;
        wolf::graphics::w_uniform<compute_unifrom_x32>*         unifrom_x32;
        wolf::graphics::w_uniform<compute_unifrom_x64>*         unifrom_x64;
        wolf::graphics::w_uniform<compute_unifrom_x128>*        unifrom_x128;
        wolf::graphics::w_uniform<compute_unifrom_x256>*        unifrom_x256;

        wolf::graphics::w_buffer                                instance_buffer;

        wolf::graphics::w_buffer                                lod_levels_buffers;
        
        wolf::graphics::w_pipeline                              pipeline;
        wolf::graphics::w_command_buffers                       command_buffers;
        VkSemaphore                                             semaphore = 0;

        void release()
        {
            SAFE_RELEASE(this->unifrom_x1);
            SAFE_RELEASE(this->unifrom_x2);
            SAFE_RELEASE(this->unifrom_x4);
            SAFE_RELEASE(this->unifrom_x8);
            SAFE_RELEASE(this->unifrom_x16);
            SAFE_RELEASE(this->unifrom_x32);
            SAFE_RELEASE(this->unifrom_x64);
            SAFE_RELEASE(this->unifrom_x128);
            SAFE_RELEASE(this->unifrom_x256);

            this->instance_buffer.release();
            this->lod_levels_buffers.release();
            this->pipeline.release();
            this->command_buffers.release();
        }

    } cs;

    struct
    {
        wolf::graphics::w_buffer                                indirect_draw_commands_buffer;
        std::vector<VkDrawIndexedIndirectCommand>               indirect_draw_commands;
        wolf::graphics::w_buffer                                indirect_draw_count_buffer;

        void release()
        {
            this->indirect_draw_commands_buffer.release();
            this->indirect_draw_count_buffer.release();
            this->indirect_draw_commands.clear();

        }
    } indirect;

    struct
    {
        uint32_t                                                draw_count;// Total number of indirect draw counts to be issued
        uint32_t                                                lod_count[MAX_LOD_LEVEL + 1];// Statistics for number of draws per LOD level (written by compute shader)
    } indirect_status;
};

#endif
