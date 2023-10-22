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
    private readonly CoroutineSuspensionPointsFlags _coroutineSuspensionPoint;

    /// <inheritdoc />
    bool ICoroutineSuspendor.Step(CoroutineSuspensionPointIndex suspensionPoint)
    {
        return false;
    }

    /// <inheritdoc />
    CoroutineSuspensionPointsFlags ICoroutineSuspendor.SuspensionPoints => _coroutineSuspensionPoint;

    private WaitForLoop(CoroutineSuspensionPointIndex coroutineSuspensionPoint)
    {
        _coroutineSuspensionPoint = (CoroutineSuspensionPointsFlags)(1 << (int)coroutineSuspensionPoint);
    }

    public static WaitForLoop Update { get; } = new(CoroutineSuspensionPointIndex.Update);
    public static WaitForLoop LateUpdate { get; } = new(CoroutineSuspensionPointIndex.LateUpdate);
    public static WaitForLoop FixedUpdate { get; } = new(CoroutineSuspensionPointIndex.FixedUpdate);
    public static WaitForLoop LateFixedUpdate { get; } = new(CoroutineSuspensionPointIndex.LateFixedUpdate);
}
