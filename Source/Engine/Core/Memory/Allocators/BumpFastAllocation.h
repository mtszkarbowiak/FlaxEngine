// Copyright (c) 2012-2024 Wojciech Figat. All rights reserved.

#pragma once

#include "Engine/Core/Memory/Allocation.h"
#include "Engine/Core/Memory/MemoryUtils.h"

// This flag enables safety checks for freed memory.
// #define ASSERT_SAFE_BUMP_ALLOC

#ifdef ASSERT_SAFE_BUMP_ALLOC
#include "Engine/Core/Log.h"
#include "Engine/Core/Collections/HashSet.h"
#endif

/// <summary>
/// Memory allocation policy with very fast allocation and deallocation.
/// It uses a bump allocator: Memory blocks are placed one after another and require manual reset.
/// This version is NOT thread-safe. There are no synchronization mechanisms which could slow down the allocation.
/// </summary>
template<typename BackupAllocation = DefaultAllocation>
class BumpFastAllocation
{
public:
    enum { HasSwap = false };

    class Context
    {
    private:
        byte* _data = nullptr;
        byte* _end = nullptr;
        byte* _bound = nullptr;

#ifdef ASSERT_SAFE_BUMP_ALLOC
        HashSet<byte*> _allocations;
#endif

    public:
        Context() = delete;

        /// <summary>
        /// Initializes the allocation context used to allocate memory.
        /// </summary>
        /// <param name="capacity">The size of the memory block in bytes.</param>
        /// <param name="alignment">The alignment of the memory block.</param>
        explicit Context(
            const uintptr capacity,
            const uintptr alignment = sizeof(byte*)
        )
        {
            _data = static_cast<byte*>(Allocator::Allocate(capacity, alignment));
            _bound = _data;
            _end = _data + capacity;
        }

        /// <summary> Copying allocation context is not allowed to prevent pointer decay. </summary>
        Context(const Context&) = delete;

        /// <summary> Moving allocation context is not allowed to prevent pointer decay. </summary>
        Context(Context&& other) = delete;

        /// <summary> Copying allocation context is not allowed to prevent pointer decay. </summary>
        auto operator=(const Context&)->Context & = delete;

        /// <summary> Moving allocation context is not allowed to prevent pointer decay. </summary>
        auto operator=(Context&&)->Context & = delete;

        /// <summary> Returns the current amount of memory used by all allocations. </summary>
        FORCE_INLINE uintptr GetUsed() const
        {
            return static_cast<uintptr>(_bound - _data);
        }

        /// <summary> Returns the size of allocated memory block. </summary>
        FORCE_INLINE uintptr GetCapacity() const
        {
            return static_cast<uintptr>(_end - _data);
        }

        ~Context()
        {
            if (_data != nullptr)
                Allocator::Free(_data);
        }

        /// <summary>
        /// Tries to allocate memory of the specified size and alignment.
        /// </summary>
        /// <returns>Pointer to the allocated memory or <c>nullptr</c> if the allocation failed.</returns>
        auto BumpAllocate(const uintptr size, const uintptr alignment = sizeof(byte*)) -> void*
        {
            // Calculate the aligned address.
            byte* aligned = MemoryUtils::Align(_bound, alignment);

            // Calculate the new bound.
            byte* newBound = aligned + size;

            // Check if the allocation fits in the memory block.
            if (newBound > _end)
            {
                // If not, we failed to allocate the memory :(
                return nullptr;
            }

            // Update the bound.
            _bound = newBound;
            return aligned;
        }

#ifdef ASSERT_SAFE_BUMP_ALLOC
    private:
        HashSet<void*> _allocations;

    public:
        void MarkFreed(void* pointer)
        {
            if (_allocations.Contains(pointer))
            {
                _allocations.Remove(pointer);
            }
        }
#endif

        /// <summary>
        /// Resets the pointer to the beginning of the memory block.
        /// </summary>
        /// <param name="clear">If <c>true</c> the memory will be zeroed.</param>
        /// <remarks>
        /// This method assumes, that all memory allocated from this context is no longer needed.
        /// </remarks>
        void Reset(const bool clear = false)
        {
            if (clear)
            {
                Platform::MemoryClear(_data, GetUsed());
            }

#ifdef ASSERT_SAFE_BUMP_ALLOC
            if (_allocations.Count() > 0)
            {
                LOG(Error, "Memory leak detected! Count: {}", _allocations.Count());
                CRASH;
            }
#endif

            // Just move the pointer to the beginning of the memory block.
            _bound = _data;
        }
    };

    template<typename T>
    class Data
    {
    private:
        using BackupData = typename BackupAllocation::template Data<T>;

        Context* _bumpContext = nullptr;
        union
        {
            T* _bumpData = nullptr;
            BackupData _backupData;
        };

    public:
        /// <summary>
        /// Initializes BumpFastAllocation with backup allocation.
        /// </summary>
        /// <summary>
        /// This constructor should be barely used.
        /// It is used only to allow default initialization of collections.
        /// </summary>
        FORCE_INLINE Data()
            : _bumpContext(nullptr)
            , _bumpData()
        {            
        }

        /// <summary>
        /// Initializes BumpFastAllocation using provided context.
        /// </summary>
        FORCE_INLINE explicit Data(Context& context)
            : _bumpContext(&context)
            , _bumpData(nullptr)
        {
            ASSERT_LOW_LAYER(&context != nullptr);
        }

        FORCE_INLINE T* Get()
        {
            return _bumpContext != nullptr ? _bumpData : _backupData.Get();
        }

        FORCE_INLINE const T* Get() const
        {
            return _bumpContext != nullptr ? _bumpData : _backupData.Get();
        }

        FORCE_INLINE int32 CalculateCapacityGrow(int32 capacity, const int32 minCapacity) const
        {
            if (capacity < minCapacity)
                capacity = minCapacity;

            return capacity < 8
                ? 8
                : MemoryUtils::NextPow2(capacity);
        }

        FORCE_INLINE void Allocate(const uintptr capacity)
        {
            if (_bumpContext == nullptr)
            {
                // If necessary, just use the backup allocation.
                _backupData.Allocate(capacity);
            }
            else
            {
                // AllocationData can be used to manipulate only one allocation at a time.
                ASSERT_LOW_LAYER(_bumpData == nullptr);

                _bumpData = static_cast<T*>(_bumpContext->BumpAllocate(capacity * sizeof(T), alignof(T)));

                if (_bumpData == nullptr)
                {
                    // If the allocation failed, we need to use the backup allocation.
                    // But first, we need to initialize it.

                    // Bump allocation context and the pointer are not needed anymore.
                    _bumpContext = nullptr;
                    new (&_backupData) BackupData();

                    _backupData.Allocate(capacity);

                    if (_bumpData == nullptr)
                        OUT_OF_MEMORY;
                }
            }
        }

        FORCE_INLINE void Relocate(const uintptr capacity, const uintptr oldCount, const uintptr newCount)
        {
            // The author of this code wasn't sure what is supposed to happen here.
            // The idea of 'relocation' isn't very C++'ish after all...

            // Let's check if someone isn't trying to init objects which are not supposed to be initialized.
            ASSERT(newCount <= capacity);

            if (_bumpContext == nullptr) 
            {
                // If necessary, just use the backup allocation.
                _backupData.Relocate(capacity, oldCount, newCount);
                // We assume that the backup allocation takes care of everything. Thus we can return.
                return;
            }

            // If less memory is required, we have to only deal with destructed objects. (Ugh, that's a bit of a mess...)
            if (capacity <= oldCount)
            {
                const auto* begin = _bumpData + newCount;
                const int32 count = static_cast<int32>(oldCount - newCount);
                Memory::DestructItems(begin, count);
                return;
            }

            // Let's prepare the new memory block. (It may happen to be _bumpData, but it's not guaranteed.)
            T* newData;

            // Let's go! Let's try to allocate the memory using the shiny bump allocator.
            _bumpData = static_cast<T*>(_bumpContext->BumpAllocate(capacity * sizeof(T), alignof(T)));

            // Oopsy daisy! We failed to allocate the memory. We have to use the backup allocation.
            if (_bumpData == nullptr)
            {
                // But first, we need to initialize it.

                // Bump allocation context and the pointer are not needed anymore. The context is set to null.
                _bumpContext = nullptr;
                new (&_backupData) BackupData();

                _backupData.Allocate(capacity);
                // Warning: If this line is executed, _bumpData must never be used again!!!

                newData = _backupData.Get();
            }
            else 
            {
                newData = _bumpData;
            }

            // We are definitely not ending the job here.

            //TODO(mtszkarbowiak) Implement the rest of the method.
        }

        FORCE_INLINE void Free()
        {
            if (_bumpContext == nullptr) // If the context is null, free the backup allocation.
            {
               _backupData.Free();
            }

#ifdef ASSERT_SAFE_BUMP_ALLOC
            else
            {
                _bumpContext->MarkFreed(_bumpData);
            }
#endif
        }

        FORCE_INLINE void Swap(Data& other)
        {
            CRASH
            //TODO(mtszkarbowiak) Implement swap.
            // Note: It will be easier to implement move operator for ALL allocations instead.
        }
    };
};
