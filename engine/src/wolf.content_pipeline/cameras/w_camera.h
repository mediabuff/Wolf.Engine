/*
	Project			 : Wolf Engine. Copyright(c) Pooya Eimandar (http://PooyaEimandar.com) . All rights reserved.
	Source			 : Please direct any bug to https://github.com/WolfSource/Wolf.Engine/issues
	Website			 : http://WolfSource.io
	Name			 : w_camera.h
	Description		 : The base class for camera
	Comment          :
*/

#ifndef __W_CAMERA_H__
#define __W_CAMERA_H__

#include "w_cpipeline_export.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/matrix.hpp>
#include <glm/vec3.hpp>
#include <msgpack.hpp>

namespace wolf
{
    namespace content_pipeline
    {
        typedef glm::tmat4x4<float, glm::precision::highp> mat4x4_p;

        class w_camera
        {
            enum w_camera_type { target, free, third_person };

        public:
            WCP_EXP w_camera();
            WCP_EXP virtual ~w_camera();

            //Update the view matrix
            WCP_EXP void update_view();
            //Update the projection matrix
            WCP_EXP void update_projection();
            //Update frustum
            WCP_EXP void update_frustum();

#pragma region Getters

            //Get name of camera
            WCP_EXP std::string get_name() const { return this->_name; };
            //Get camera target name
            WCP_EXP std::string get_camera_target_name() const { return this->_camera_target_name; }
            //Get field of view
            WCP_EXP float get_field_of_view() { return this->_field_of_view; }
            //Get field of view
            WCP_EXP float get_field_of_view() const { return this->_field_of_view; }
            //Get aspect ratio of the camera
            WCP_EXP float get_aspect_ratio() const { return this->_aspect_ratio; }
            //Get near plane of the camera
            WCP_EXP float get_near_plane() const { return this->_near_plane; }
            //Get far plane of the camera
            WCP_EXP float get_far_plane() const { return this->_far_plane; }
            //Get view matrix of the camera
            WCP_EXP mat4x4_p get_view() const { return this->_view; }
            //Get projection matrix of the camera
            WCP_EXP mat4x4_p get_projection() const { return this->_projection; }
            //Get projection * view matrix of the camera
            WCP_EXP mat4x4_p get_projection_view() const;
            //Get projection matrix of the camera
            WCP_EXP std::array<glm::vec4, 6> get_frustum_plans() const { return this->_frustum_planes; }
            //Get position of the camera
            WCP_EXP glm::vec3 get_translate() const { return glm::vec3(this->_translate[0], this->_translate[1], this->_translate[2]); }
            //Get interest of the camera
            WCP_EXP glm::vec3 get_interest() const { return glm::vec3(this->_interest[0], this->_interest[1], this->_interest[2]); }

#pragma region 

#pragma region Setters

            //Set name of camera
            WCP_EXP void set_name(const std::string& pValue);
            //Set camera target name
            WCP_EXP void set_camera_target_name(_In_z_ const std::string& pValue);
            //Set field of view
            WCP_EXP void set_field_of_view(_In_ const float pValue);
            //Set near plan
            WCP_EXP void set_near_plan(_In_ const float pValue);
            //Set far plan
            WCP_EXP void set_far_plan(_In_ const float pValue);
            //Set aspect ratio of the camera
            WCP_EXP void set_aspect_ratio(_In_ const float pAspectRatio);
            //Set position of the camera
            WCP_EXP void set_translate(_In_ const float pX, _In_ const float pY, _In_ const float pZ);
            //Set position of the camera
            WCP_EXP void set_translate(_In_ const glm::vec3 pInterest);
            //Set camera interest
            WCP_EXP void set_interest(_In_ const float pX, _In_ const float pY, _In_ const float pZ);
            //Set camera interest
            WCP_EXP void set_interest(_In_ const glm::vec3 pInterest);

#pragma endregion

            MSGPACK_DEFINE(
                _name, _camera_target_name, _field_of_view,
                _near_plane, _far_plane, _up,
                _translate, _interest);

        protected:
            w_camera_type               _type;
            std::string		            _name;
            std::string                 _camera_target_name;
            float			            _field_of_view;
            float			            _aspect_ratio;
            float			            _near_plane;
            float			            _far_plane;
            float		                _up[3];
            float		                _translate[3];
            float	    	            _interest[3];
            mat4x4_p		            _view;
            mat4x4_p		            _projection;
            std::array<glm::vec4, 6>    _frustum_planes;
            bool                        _z_up;
        };
    }
}

#endif
