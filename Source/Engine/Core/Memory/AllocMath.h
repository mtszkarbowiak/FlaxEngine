// Copyright (c) 2012-2024 Wojciech Figat. All rights reserved.

#pragma once

// Round up to the next power of 2 and multiply by 2 (http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2)

/// <summary>
/// Rounds up the given number to the next power of two.
/// This version is for 8-bit numbers.
/// </summary>
/// <param name="n"> Reference to the number to round up.</param>
/// <remarks>
/// Macros are used instead of functions to manually inline the code.
/// Be very careful! Binding with non-pure expressions can cause unexpected behavior (e.g. ROUND_UP_TO_POWER_OF_TWO_16(x++)).
/// </remarks>
#define ROUND_UP_TO_POWER_OF_TWO_8(n) static_assert(sizeof(n) == 2, "The number must be 16-bit."); \
    --(n);            \
    (n) |= (n) >> 1;  \
    (n) |= (n) >> 2;  \
    (n) |= (n) >> 4;  \
    ++(n);

/// <summary>
/// Rounds up the given number to the next power of two.
/// This version is for 16-bit numbers.
/// </summary>
/// <param name="n"> Reference to the number to round up.</param>
/// <remarks>
/// Macros are used instead of functions to manually inline the code.
/// Be very careful! Binding with non-pure expressions can cause unexpected behavior (e.g. ROUND_UP_TO_POWER_OF_TWO_16(x++)).
/// </remarks>
#define ROUND_UP_TO_POWER_OF_TWO_16(n) static_assert(sizeof(n) == 2, "The number must be 16-bit."); \
    --(n);            \
    (n) |= (n) >> 1;  \
    (n) |= (n) >> 2;  \
    (n) |= (n) >> 4;  \
    (n) |= (n) >> 8;  \
    ++(n);

/// <summary>
/// Rounds up the given number to the next power of two.
/// This version is for 32-bit numbers.
/// </summary>
/// <param name="n"> Reference to the number to round up.</param>
/// <remarks>
/// Macros are used instead of functions to manually inline the code.
/// Be very careful! Binding with non-pure expressions can cause unexpected behavior (e.g. ROUND_UP_TO_POWER_OF_TWO_16(x++)).
/// </remarks>
#define ROUND_UP_TO_POWER_OF_TWO_32(n) static_assert(sizeof(n) == 4, "The number must be 32-bit."); \
    --(n);            \
    (n) |= (n) >> 1;  \
    (n) |= (n) >> 2;  \
    (n) |= (n) >> 4;  \
    (n) |= (n) >> 8;  \
    (n) |= (n) >> 16; \
    ++(n);

/// <summary>
/// Rounds up the given number to the next power of two.
/// This version is for 64-bit numbers.
/// </summary>
/// <param name="n"> Reference to the number to round up.</param>
/// <remarks>
/// Macros are used instead of functions to manually inline the code.
/// Be very careful! Binding with non-pure expressions can cause unexpected behavior (e.g. ROUND_UP_TO_POWER_OF_TWO_16(x++)).
/// </remarks>
#define ROUND_UP_TO_POWER_OF_TWO_64(n) static_assert(sizeof(n) == 8, "The number must be 64-bit."); \
    --(n);            \
    (n) |= (n) >> 1;  \
    (n) |= (n) >> 2;  \
    (n) |= (n) >> 4;  \
    (n) |= (n) >> 8;  \
    (n) |= (n) >> 16; \
    (n) |= (n) >> 32; \
    ++(n);
