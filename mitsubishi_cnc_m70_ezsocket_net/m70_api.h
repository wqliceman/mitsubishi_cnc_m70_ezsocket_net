/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2022-2026 wqliceman
 * GitHub: iceman
 * Email: wqliceman@gmail.com
 *
 * This file is part of the Mitsubishi CNC M70 EZSocket communication library.
 * See the LICENSE file in the project root for full license information.
 */

#ifndef __H_M70_API_H__
#define __H_M70_API_H__

#if defined(_WIN32) || defined(__CYGWIN__)
#if defined(M70_BUILD_DLL)
#define M70_API __declspec(dllexport)
#elif defined(M70_USE_DLL)
#define M70_API __declspec(dllimport)
#else
#define M70_API
#endif
#else
#define M70_API
#endif

#endif // __H_M70_API_H__