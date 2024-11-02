// Copyright (c) 2012-2024 Wojciech Figat. All rights reserved.

#pragma once

#include "Engine/Core/Memory/Allocation.h"
#include "Engine/Core/Memory/MemoryUtils.h"
#include "Engine/Threading/Threading.h"

// This flag enables safety checks for freed memory.
// #define ASSERT_SAFE_BUMP_ALLOC

#ifdef ASSERT_SAFE_BUMP_ALLOC
#include "Engine/Core/Log.h"
#include "Engine/Core/Collections/HashSet.h"
#endif

/// <summary>
/// Memory allocation policy with very fast allocation and deallocation.
/// It uses a bump allocator: Memory blocks are placed one after another and require manual reset.
/// This version is thread-safe. It uses atomic operations and locks to synchronize access to the memory block.
/// </summary>
class BumpSyncAllocation
{
public:
    enum { HasSwap = true };

    class Context
    {
    private:
        byte* _data = nullptr;
        byte* _end = nullptr;
        byte* _bound = nullptr;

#ifdef ASSERT_SAFE_BUMP_ALLOC
        CriticalSection _mutex;
        HashSet<byte*> _allocations;
#endif
        
    public:
        Context() = delete;
    };
};
