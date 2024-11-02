// Copyright (c) 2012-2024 Wojciech Figat. All rights reserved.

#pragma once

#include "Engine/Core/Memory/Allocators/BumpSyncAllocation.h"

/// <summary>
/// Memory allocation policy with very fast allocation and deallocation.
/// It uses a synchronized bump allocator, with reset done automatically by the game service.
/// Warning: The allocation is assumed to never live longer than a single frame.
/// This version is thread-safe.
/// </summary>
class FrameAllocation
{
    //TODO(mtszkarbowiak)
};
