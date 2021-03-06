/*
	Project			 : Wolf Engine. Copyright(c) http://PooyaEimandar.com . All rights reserved. https://github.com/PooyaEimandar/WolfEngine
	Website			 : http://WolfStudio.co
	Name             : PCH.h
	Description		 : Pre-Compiled header for System
	Comment          :
*/

#pragma once

#include "targetver.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <memory>

#ifndef __WIN32
#define __WIN32
#endif

#pragma comment(lib, "Wolf.System.Win32.lib")
#pragma comment(lib, "Wolf.DirectX.Win32.lib")
