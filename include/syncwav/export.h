#pragma once

#ifdef _WIN32
  #ifdef SWAV_BUILD_DLL
    #define SWAV_API __declspec(dllexport)
  #else
    #define SWAV_API __declspec(dllimport)
  #endif
#else
  #define SWAV_API
#endif
