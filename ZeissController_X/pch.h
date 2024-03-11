// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H
#define _CRT_SECURE_NO_WARNINGS
#define _REPORT_TO_CONSOLE_
//#define _REPORT_TO_CONSOLE_FUNCTION_NAMES_


//Personal #defines
#ifdef _REPORT_TO_CONSOLE_
#define PRINT(x) std::cout << x << std::endl //printf(x);
#define WPRINT(x) std::wcout << x << std::endl //wprintf(x)
#else
#define PRINT(x)  //printf(x);
#define WPRINT(x) //wprintf(x)
#endif
#ifdef _REPORT_TO_CONSOLE_FUNCTION_NAMES_
#define PRINTD(x) printf(x);
#define WPRINTD(x) wprintf(x)
#else
#define PRINTD(x)  //printf(x);
#define WPRINTD(x) //wprintf(x)
#endif

#define ZM(x) ZeroMemory(&x, sizeof(x))
#define MB(x) MessageBox(NULL, x, NULL, NULL)
#define WAIT() printf("Press ENTER to continue...\n"); std::cin.get()
#define SAFE_RELEASE(x) if(x) delete x; x = nullptr
// add headers that you want to pre-compile here
#include "framework.h"
#include "afxdialogex.h"

#import "CZEMapi.ocx" named_guids //no_namespace no_smart_pointers raw_interfaces_only raw_native_types no_implementation named_guids
//#include "Release/czemapi.tli"

// For the console
#include <iostream>

// STL DataStructures
#include <Vector>
#include <string>
#include <map>
#include <unordered_map>
#include <memory.h>
#include <algorithm>
#include <cmath>
#include <thread>
#include <variant>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <format>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

// For the GUI
#include <windows.h>	
#include "resource.h"

#define _ANG_TO_KEY_MULTIPLIER_ 10
#define STEMRESX	1024
#define STEMRESY	768
#define DESKTOSTEMX 128
#define DESKTOSTEMY 137
//#define DESKTOSTEMY 109 Sometimes its this, I don't understand ^^

//1280x1024


// Personal Headers
#include "CTimer.h"
#include "CWriteFile.h"
#include "CLinearMover.h"
#include "CImageManager.h"
#include "ZeissController_X.h"
#include "CTEMControlManager.h"
#include "CTimepix.h"
#include "CZeissStage.h"
#include "CDataCollection.h"
#include "CZeissQuickControl.h"
#include "CPostDataCollection.h"



using namespace std::chrono_literals;

#endif //PCH_H
