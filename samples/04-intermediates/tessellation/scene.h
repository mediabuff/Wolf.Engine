/*
	Project			 : Wolf Engine. Copyright(c) Pooya Eimandar (http://PooyaEimandar.com) . All rights reserved.
	Source			 : Please direct any bug to https://github.com/WolfSource/Wolf.Engine/issues
	Website			 : http://WolfSource.io
	Name			 : scene.h
	Description		 : The main class of your game
	Comment          :
*/

#ifndef __SCENE_H__
#define __SCENE_H__

#include <w_cpipeline_scene.h>
#include <w_framework/w_game.h>
#include <w_framework/w_model.h>
#include <w_graphics/w_shader.h>
#include <w_graphics/w_uniform.h>
#include <w_graphics/w_command_buffers.h>
#include <w_graphics/w_render_pass.h>
#include <w_graphics/w_frame_buffers.h>
#include <w_graphics/w_pipeline.h>

class scene : public wolf::framework::w_game
{
public:
#if defined(__WIN32)
	W_EXP scene(_In_z_ const std::wstring& pRunningDirectory, _In_z_ const std::wstring& pAppName);
#elif defined(__UWP)
	W_EXP scene(_In_z_ const std::wstring& pAppName);
#else
	W_EXP scene(_In_z_ const std::string& pRunningDirectory, _In_z_ const std::string& pAppName);
#endif
    virtual ~scene();
    
    /*
        Allows the game to perform any initialization and it needs to before starting to run.
        Calling Game::Initialize() will enumerate through any components and initialize them as well.
        The parameter pOutputWindowsInfo represents the information of output window(s) of this game.
     */
    void initialize(_In_ std::map<int, std::vector<w_window_info>> pOutputWindowsInfo) override;
    
    //The function "Load()" will be called once per game and is the place to load all of your game assets.
    void load() override;
    
    //This is the place where allows the game to run logic such as updating the world, checking camera, collisions, physics, input, playing audio and etc.
    void update(_In_ const wolf::system::w_game_time& pGameTime) override;

    //Render on all graphics devices
    HRESULT render(_In_ const wolf::system::w_game_time& pGameTime) override;
        
    //This is called when the window game should resized. pIndex is the index of window.
    void on_window_resized(_In_ UINT pIndex) override;
    
    //This is called when the we lost graphics device.
    void on_device_lost() override;

#ifdef __WIN32
    //handle user inputs
    HRESULT on_msg_proc(
        _In_ const HWND pHWND, 
        _In_ const UINT pMessage, 
        _In_ const WPARAM pWParam, 
        _In_ const LPARAM pLParam) override;
#endif

    //Release will be called once per game and is the place to unload assets and release all resources
    ULONG release() override;
    
private:
    
#pragma pack(push,1)
    struct tessellation_level_unifrom
    {
        float tess_level_uniform = 1.0f;
    };
#pragma pack(pop)

#pragma pack(push,1)
    struct world_view_projection_unifrom
    {
        glm::mat4 view_projection = glm::mat4(1);//identity
        glm::mat4 world = glm::mat4(1);//identity
    };
#pragma pack(pop)
    
#pragma pack(push,1)
    struct color_unifrom
    {
        glm::vec4 color = glm::vec4(1, 1, 1, 1);
    };
#pragma pack(pop)

    wolf::content_pipeline::w_camera                               _camera;
    wolf::graphics::w_command_buffers                              _command_buffers;
    wolf::graphics::w_render_pass                                  _render_pass;
    wolf::graphics::w_frame_buffers                                _frame_buffers;


    struct model
    {
        wolf::framework::w_model<>*                                 model_meshes = nullptr;
        wolf::graphics::w_uniform<tessellation_level_unifrom>       tess_level_unifrom;
        wolf::graphics::w_uniform<world_view_projection_unifrom>    wvp_unifrom;
        wolf::graphics::w_uniform<color_unifrom>                    color_unifrom;
        wolf::graphics::w_shader*                                   shader = nullptr;
        wolf::graphics::w_pipeline*                                 pipeline = nullptr;
        bool                                                        has_instances = false;

        ULONG release()
        {
            SAFE_RELEASE(this->model_meshes);
            SAFE_RELEASE(this->shader);
            SAFE_RELEASE(this->pipeline);
            this->wvp_unifrom.release();
            this->color_unifrom.release();
            return 1;
        }
    };

    std::vector<model*>                                             _models;
};

#endif
