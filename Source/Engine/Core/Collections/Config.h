// Copyright (c) 2012-2024 Wojciech Figat. All rights reserved.

#pragma once

#include "Engine/Platform/Defines.h"

/// <summary>
/// Default capacity for the arrays.
/// </summary>
#define ARRAY_DEFAULT_CAPACITY 4

/// <summary>
/// Default capacity for the dictionaries.
/// </summary>
#ifndef DICTIONARY_DEFAULT_CAPACITY
#if PLATFORM_DESKTOP
#define DICTIONARY_DEFAULT_CAPACITY 256
#else
#define DICTIONARY_DEFAULT_CAPACITY 64
#endif
#endif

/// <summary>
/// Default slack space divider for the dictionaries.
/// </summary>
#define DICTIONARY_DEFAULT_SLACK_SCALE 3

/// <summary>
/// Function for dictionary that tells how change hash index during iteration (size param is a buckets table size).
/// </summary>
#define DICTIONARY_PROB_FUNC(size, numChecks) (numChecks)
//#define DICTIONARY_PROB_FUNC(size, numChecks) (1)
