// Copyright (c) 2012-2024 Wojciech Figat. All rights reserved.

#pragma once

#include "Engine/Platform/Platform.h"
#include "Engine/Core/Memory/Allocation.h"
#include "Engine/Core/Math/Math.h"

/// <summary>
/// Template for dynamic array with variable capacity that stores the bit values.
/// </summary>
template<typename WannaBeAlloc = HeapAllocation> //TODO Use template argument allocation.
API_CLASS(InBuild) class BitArray
{
    friend BitArray;
public:
    using BlockType = uint64;
    using AllocationType = HeapAllocation; // Disable custom allocation for now.
    using AllocationData = typename AllocationType::template Data<BlockType>;

private:
    int32 _bitCount;
    int32 _bitCapacity;
    AllocationData _allocation;

    FORCE_INLINE static int32 ToBlockCount(const int32 size)
    {
        return Math::DivideAndRoundUp<int32>(size, sizeof(BlockType));
    }

    FORCE_INLINE static int32 ToBlockCapacity(const int32 size)
    {
        return Math::Max<int32>(Math::DivideAndRoundUp<int32>(size, sizeof(BlockType)), 1);
    }

public:
    /// <summary>
    /// Initializes a new instance of the <see cref="BitArray"/> class.
    /// </summary>
    FORCE_INLINE BitArray()
        : _bitCount(0)
        , _bitCapacity(0)
    {
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="BitArray"/> class.
    /// </summary>
    /// <param name="capacity">The initial capacity.</param>
    explicit BitArray(const int32 capacity)
        : _bitCount(0)
        , _bitCapacity(capacity)
    {
        if (capacity > 0)
            _allocation.Allocate(ToBlockCapacity(capacity));
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="BitArray"/> class.
    /// </summary>
    /// <param name="other">The other collection to copy.</param>
    BitArray(const BitArray& other) noexcept
    {
        _bitCount = _bitCapacity = other.Count();
        if (_bitCapacity > 0)
        {
            const int32 itemsCapacity = ToBlockCapacity(_bitCapacity);
            _allocation.Allocate(itemsCapacity);
            Platform::MemoryCopy(GetBlocks(), other.GetBlocks(), itemsCapacity * sizeof(BlockType));
        }
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="BitArray"/> class.
    /// </summary>
    /// <param name="other">The other collection to copy.</param>
    template<typename OtherAllocationType = AllocationType>
    explicit BitArray(const BitArray<OtherAllocationType>& other) noexcept
    {
        _bitCount = _bitCapacity = other.Count();
        if (_bitCapacity > 0)
        {
            const int32 itemsCapacity = ToBlockCapacity(_bitCapacity);
            _allocation.Allocate(itemsCapacity);
            Platform::MemoryCopy(GetBlocks(), other.GetBlocks(), itemsCapacity * sizeof(BlockType));
        }
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="BitArray"/> class.
    /// </summary>
    /// <param name="other">The other collection to move.</param>
    FORCE_INLINE BitArray(BitArray&& other) noexcept
    {
        _bitCount = other._bitCount;
        _bitCapacity = other._bitCapacity;
        other._bitCount = 0;
        other._bitCapacity = 0;
        _allocation.Swap(other._allocation); // WHAT?!
    }

    /// <summary>
    /// The assignment operator that deletes the current collection of items and the copies items from the other array.
    /// </summary>
    /// <param name="other">The other collection to copy.</param>
    /// <returns>The reference to this.</returns>
    BitArray& operator=(const BitArray& other) noexcept
    {
        if (this != &other)
        {
            if (_bitCapacity < other._bitCount)
            {
                _allocation.Free();
                _bitCapacity = other._bitCount;
                const int32 itemsCapacity = ToBlockCapacity(_bitCapacity);
                _allocation.Allocate(itemsCapacity);
                Platform::MemoryCopy(GetBlocks(), other.GetBlocks(), itemsCapacity * sizeof(BlockType));
            }
            _bitCount = other._bitCount;
        }
        return *this;
    }

    /// <summary>
    /// The move assignment operator that deletes the current collection of items and the moves items from the other array.
    /// </summary>
    /// <param name="other">The other collection to move.</param>
    /// <returns>The reference to this.</returns>
    BitArray& operator=(BitArray&& other) noexcept
    {
        if (this != &other)
        {
            _allocation.Free();
            _bitCount = other._bitCount;
            _bitCapacity = other._bitCapacity;
            other._bitCount = 0;
            other._bitCapacity = 0;
            _allocation.Swap(other._allocation); // WHAT?!
        }
        return *this;
    }

    /// <summary>
    /// Finalizes an instance of the <see cref="BitArray"/> class.
    /// </summary>
    ~BitArray()
    {
    }

public:
    /// <summary>
    /// Gets the pointer to the bits storage data (linear allocation).
    /// </summary>
    FORCE_INLINE BlockType* GetBlocks()
    {
        return _allocation.Get();
    }

    /// <summary>
    /// Gets the pointer to the bits storage data (linear allocation).
    /// </summary>
    FORCE_INLINE const BlockType* GetBlocks() const
    {
        return _allocation.Get();
    }

    /// <summary>
    /// Gets the amount of the items in the collection.
    /// </summary>
    FORCE_INLINE int32 Count() const
    {
        return _bitCount;
    }

    /// <summary>
    /// Gets the amount of the items that can be contained by collection without resizing.
    /// </summary>
    FORCE_INLINE int32 Capacity() const
    {
        return _bitCapacity;
    }

    /// <summary>
    /// Returns true if collection isn't empty.
    /// </summary>
    FORCE_INLINE bool HasItems() const
    {
        return _bitCount != 0;
    }

    /// <summary>
    /// Returns true if collection is empty.
    /// </summary>
    FORCE_INLINE bool IsEmpty() const
    {
        return _bitCount == 0;
    }

    /// <summary>
    /// Gets the item at the given index.
    /// </summary>
    /// <param name="index">The index of the item.</param>
    /// <returns>The value of the item.</returns>
    FORCE_INLINE bool operator[](const int32 index) const
    {
        return Get(index);
    }

    /// <summary>
    /// Gets the item at the given index.
    /// </summary>
    /// <param name="index">The index of the item.</param>
    /// <returns>The value of the item.</returns>
    FORCE_INLINE bool Get(const int32 index) const
    {
        ASSERT(index >= 0 && index < _bitCount);

        const int32 blockIndex = index / sizeof(BlockType);
        const int32 bitIndex = index % sizeof(BlockType);
        const BlockType& block = _allocation.Get()[blockIndex];
        const BlockType mask = BlockType{ 1 } << bitIndex;

        return (block & mask) != 0;
    }

    /// <summary>
    /// Sets the item at the given index.
    /// </summary>
    /// <param name="index">The index of the item.</param>
    /// <param name="value">The value to set.</param>
    FORCE_INLINE void Set(const int32 index, const bool value)
    {
        ASSERT(index >= 0 && index < _bitCount);

        const int32 blockIndex = index / sizeof(BlockType);
        const int32 bitIndex = index % sizeof(BlockType);
        const BlockType bitMask = BlockType{ 1 } << bitIndex;
        BlockType& block = _allocation.Get()[blockIndex];

        // Collapse bool value into 0 or 1 of BlockType.
        // Then flip the sign, creating a mask: 0b00000... or 0b11111... .
        const BlockType toggleMask = -static_cast<BlockType>(value);

        // Now use the mask to toggle between 
        block = 
            (block & ~bitMask) // Filter the old mask out.
            | 
            (toggleMask & bitMask); // Optionally, set the new mask in.
    }

public:
    /// <summary>
    /// Clear the collection without changing its capacity.
    /// </summary>
    FORCE_INLINE void Clear()
    {
        _bitCount = 0;
    }

    /// <summary>
    /// Clear the collection and free the memory, changing its capacity to 0.
    /// </summary>
    FORCE_INLINE void ClearAndFree()
    {
        _bitCount = 0;
        _bitCapacity = 0;
        _allocation.Free();
    }

    /// <summary>
    /// Changes the capacity of the collection.
    /// </summary>
    /// <param name="capacity">The new capacity.</param>
    /// <param name="preserveContents">True if preserve collection data when changing its size, otherwise collection after resize will be empty.</param>
    void SetCapacity(const int32 capacity, const bool preserveContents = true) //TODO Remove this method.
    {
        if (capacity == _bitCapacity)
            return;
        ASSERT(capacity >= 0);
        const int32 count = preserveContents ? (_bitCount < capacity ? _bitCount : capacity) : 0;
        _allocation.Relocate(ToBlockCapacity(capacity), ToBlockCount(_bitCount), ToBlockCount(count));
        _bitCapacity = capacity;
        _bitCount = count;
    }

    /// <summary>
    /// Resizes the collection to the specified size. If the size is equal or less to the current capacity no additional memory reallocation in performed.
    /// </summary>
    /// <param name="size">The new collection size.</param>
    /// <param name="preserveContents">True if preserve collection data when changing its size, otherwise collection after resize might not contain the previous data.</param>
    void Resize(const int32 size, const bool preserveContents = true)
    {
        if (_bitCount <= size)
            EnsureCapacity(size, preserveContents);
        _bitCount = size;
    }

    /// <summary>
    /// Ensures the collection has given capacity (or more).
    /// </summary>
    /// <param name="minCapacity">The minimum capacity.</param>
    /// <param name="preserveContents">True if preserve collection data when changing its size, otherwise collection after resize will be empty.</param>
    void EnsureCapacity(const int32 minCapacity, const bool preserveContents = true)
    {
        if (_bitCapacity < minCapacity)
        {
            const int32 capacity = _allocation.CalculateCapacityGrow(ToBlockCapacity(_bitCapacity), minCapacity);
            SetCapacity(capacity, preserveContents);
        }
    }

    /// <summary>
    /// Sets all items to the given value
    /// </summary>
    /// <param name="value">The value to assign to all the collection items.</param>
    void SetAll(const bool value)
    {
        if (_bitCount != 0)
            Platform::MemorySet(_allocation.Get(), ToBlockCount(_bitCount) * sizeof(BlockType), value ? MAX_uint32 : 0);
    }

    /// <summary>
    /// Adds the specified item to the collection.
    /// </summary>
    /// <param name="item">The item to add.</param>
    void Add(const bool item)
    {
        EnsureCapacity(_bitCount + 1);
        ++_bitCount;
        Set(_bitCount - 1, item);
    }

    /// <summary>
    /// Adds the specified item to the collection.
    /// </summary>
    /// <param name="items">The items to add.</param>
    /// <param name="count">The items count.</param>
    void Add(const bool* items, const int32 count)
    {
        EnsureCapacity(_bitCount + count);
        for (int32 i = 0; i < count; ++i)
            Add(items[i]);
    }

    /// <summary>
    /// Adds the other collection to the collection.
    /// </summary>
    /// <param name="other">The other collection to add.</param>
    void Add(const BitArray& other)
    {
        EnsureCapacity(_bitCount, other.Count());
        for (int32 i = 0; i < other.Count(); ++i)
            Add(other[i]);
    }

    /// <summary>
    /// Swaps the contents of collection with the other object without copy operation. Performs fast internal data exchange.
    /// </summary>
    /// <param name="other">The other collection.</param>
    void Swap(BitArray& other)
    {
        ::Swap(_bitCount, other._bitCount);
        ::Swap(_bitCapacity, other._bitCapacity);
        _allocation.Swap(other._allocation);
    }

public:
    template<typename OtherAllocationType = AllocationType>
    bool operator==(const BitArray<OtherAllocationType>& other) const
    {
        if (_bitCount == other.Count())
        {
            for (int32 i = 0; i < _bitCount; i++)
            {
                if (!(Get(i) == other.Get(i)))
                    return false;
            }
            return true;
        }
        return false;
    }

    template<typename OtherAllocationType = AllocationType>
    bool operator!=(const BitArray<OtherAllocationType>& other) const
    {
        return !operator==(other);
    }
};
