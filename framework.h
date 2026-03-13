#pragma once
#define _CRT_SECURE_NO_WARNINGS
//#define WIN32_MIN_AND_LESS
#include<cstdint>
#include<cstdio>
#include<cstdlib>
#include<cmath>
#include<cassert>
#include<crtdbg.h>


#include<iostream>
#include<string>
#include<vector>
#include<memory>
#include<algorithm>



#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include<windows.h>

#include<windowsx.h>

#include<shellapi.h>

#include<PathCch.h>
#pragma comment(lib,"pathcch.lib")



//zlib
#include"zlib.h"
#include"zconf.h"

//cv
#include"opencv2/opencv.hpp"

//compressonator
#include"compressonator.h"

//DirectXTex
//vcpkg装出来不正常，手动拷一份得了
#include"dds.h"

