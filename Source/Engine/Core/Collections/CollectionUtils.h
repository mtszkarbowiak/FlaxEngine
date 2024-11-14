// Copyright (c) 2012-2024 Wojciech Figat. All rights reserved.

#pragma once

#include "Engine/Core/Memory/Memory.h"

class CollectionUtils final
{
    template<typename T, typename AllocationType>
    FORCE_INLINE static TEnableIfT<TIsMoveConstructibleV<
        typename AllocationType::template Data<T>>
    > MoveLinearContentImpl(
        typename AllocationType::template Data<T>& to,
        typename AllocationType::template Data<T>& from,
        const int32 fromCount,
        const int32 fromCapacity
    )
    {
        to = MoveTemp(from);
    }

    template<typename T, typename AllocationType>
    FORCE_INLINE static TEnableIfT<!TIsMoveConstructibleV< 
        typename AllocationType::template Data<T>
    >> MoveLinearContentImpl(
        typename AllocationType::template Data<T>& to,
        typename AllocationType::template Data<T>& from,
        const int32 fromCount,
        const int32 fromCapacity
    )
    {
        to.Allocate(fromCapacity);
        Memory::MoveItems(to.Get(), from.Get(), fromCount);
        Memory::DestructItems(from.Get(), fromCount);
        from.Free();
    }


public:
    /// <summary>
    /// Moves the content of the collection from one to another.
    /// </summary>
    /// <typeparam name="T">The type of the elements in the collection.</typeparam>
    /// <typeparam name="AllocationType">The type of memory allocator.</typeparam>
    /// <param name="from">The source collection allocation.</param>
    /// <param name="to">The destination collection allocation.</param>
    /// <param name="fromCount">The number of elements in the source collection. If necessary, elements [0, fromCount) may become a subject of move.</param>
    /// <param name="fromCapacity">The capacity of the source collection.</param>
    /// <remarks>
    /// This method has no knowledge of true collection capacity. Providing smaller capacity may reduce the new allocation size.
    /// Be careful not to provide a capacity smaller than the count of elements.
    /// </remarks>
    template<typename T, typename AllocationType>
    FORCE_INLINE static void MoveLinearContent(
        typename AllocationType::template Data<T>& to,
        typename AllocationType::template Data<T>& from,
        const int32 fromCount,
        const int32 fromCapacity
    )
    {
        ASSERT(fromCapacity >= fromCount);

        //TODO Manually inline this method.
        MoveLinearContentImpl<T, AllocationType>(to, from, fromCount, fromCapacity);
    }


    /// <summary>
    /// Calculates the capacity of the allocation based on the requested count of elements.
    /// </summary>
    /// <typeparam name="AllocationType">The type of memory allocator, providing information about allocation capacity limits.</typeparam>
    /// <param name="count">The requested minimal count of elements to store.</param>
    template<typename AllocationType>
    FORCE_INLINE static int32 CalculateCapacity(const int32 count)
    {
        static_assert(AllocationType::MinCapacity > 0, "The minimum capacity must be greater than zero.");
        static_assert(AllocationType::MaxCapacity >= AllocationType::MinCapacity, "The maximum capacity must be greater or equal to the minimum capacity.");

        // 1. Count of zero is a special case. It means that there should not be any allocation at all.
        // Thus, it should be handled separately and is treated as illegal argument.
        // 2. The requested capacity must never exceed the maximum capacity of the allocation.
        ASSERT((0 < count) && (count <= AllocationType::MaxCapacity));

        // By default, the capacity is just doubled.
        int32 capacity = count;
        ROUND_UP_TO_POWER_OF_TWO_32(capacity);

        if (capacity < AllocationType::MinCapacity)
            return AllocationType::MinCapacity;
        else if (capacity > AllocationType::MaxCapacity)
            return AllocationType::MaxCapacity; // We already know that count <= MaxCapacity, so this is safe.

        return capacity;
    }
};
