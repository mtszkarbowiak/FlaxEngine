// Copyright (c) 2012-2024 Wojciech Figat. All rights reserved.

#pragma once

#include "Engine/Core/Memory/Memory.h"

class CollectionUtils final
{
public:
    template<typename T, typename AllocationType>
    FORCE_INLINE static void MoveToEmpty(
        typename AllocationType::template Data<T>& to, 
        typename AllocationType::template Data<T>& from,
        const int32 fromCount, 
        const int32 fromCapacity
    )
    {
        if IF_CONSTEXPR(AllocationType::HasSwap)
            to.Swap(from);
        else
        {
            to.Allocate(fromCapacity);
            Memory::MoveItems(to.Get(), from.Get(), fromCount);
            Memory::DestructItems(from.Get(), fromCount);
            from.Free();
        }
    }
};
