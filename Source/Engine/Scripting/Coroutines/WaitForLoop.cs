#nullable enable

using FlaxEngine.Assertions;

namespace FlaxEngine;

/// <summary>
///     Waits for point in a game loop.
/// </summary>
/// <remarks>
///     To use this class, use one of the static instances.
/// </remarks>
public sealed class WaitForLoop : ICoroutineSuspendor
{
    private readonly CoroutineSuspensionPointsFlags _flags;

    /// <inheritdoc />
    bool ICoroutineSuspendor.Step(CoroutineSuspensionPointIndex index)
    {
        Assert.AreEqual(
            _flags,
            (CoroutineSuspensionPointsFlags)(1 << (int)index),
            $"{nameof(WaitForLoop)} must not step at {index}. Expected one of: {_flags}."
        );

        return false;
    }

    /// <inheritdoc />
    CoroutineSuspensionPointsFlags ICoroutineSuspendor.SuspensionPoints => _flags;

    private WaitForLoop(CoroutineSuspensionPointIndex coroutineSuspensionPoint)
    {
        _flags = (CoroutineSuspensionPointsFlags)(1 << (int)coroutineSuspensionPoint);
    }

    public static WaitForLoop Update { get; } = new(CoroutineSuspensionPointIndex.Update);
    public static WaitForLoop LateUpdate { get; } = new(CoroutineSuspensionPointIndex.LateUpdate);
    public static WaitForLoop FixedUpdate { get; } = new(CoroutineSuspensionPointIndex.FixedUpdate);
    public static WaitForLoop LateFixedUpdate { get; } = new(CoroutineSuspensionPointIndex.LateFixedUpdate);
}
