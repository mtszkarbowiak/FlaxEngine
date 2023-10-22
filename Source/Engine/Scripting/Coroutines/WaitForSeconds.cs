#nullable enable

using FlaxEngine.Assertions;

namespace FlaxEngine;

/// <summary>
///     Suspend a coroutine until a certain amount of seconds have passed. It suspend the coroutine at Update.
/// </summary>
/// <remarks>
///     To reduce GC pressure this class can be reused.
///     To do so, call <see cref="ExpectedDuration"/> and <see cref="PassedDuration"/> are exposed.
/// </remarks>
public sealed class WaitForSeconds : ICoroutineSuspendor
{
    /// <summary>
    ///		How long the coroutine should be suspended.
    /// </summary>
    public float ExpectedDuration { get; set; }

    /// <summary>
    ///		How long the coroutine has already been suspended.
    /// </summary>
    public float PassedDuration { get; set; }

    /// <summary>
    ///		Is the time scaled by the engine's time scale.
    /// </summary>
    public bool IsScaledTime { get; init; }


    /// <summary>
    ///     Sets <see cref="WaitForSeconds"/> up.
    /// </summary>
    public WaitForSeconds(float duration)
    {
        ExpectedDuration = duration;
        PassedDuration = 0;
    }

    /// <summary>
    ///     Resets the coroutine's suspension duration.
    /// </summary>
    public void Reset()
    {
        PassedDuration = 0;
    }


    bool ICoroutineSuspendor.Step(CoroutineSuspensionPointIndex suspensionPoint)
    {
        Assert.IsTrue(
          suspensionPoint == CoroutineSuspensionPointIndex.Update,
          $"{nameof(WaitForSeconds)} must not step at {suspensionPoint}."
         );

        var delta = IsScaledTime ? Time.DeltaTime : Time.UnscaledDeltaTime;

        PassedDuration += delta;
        return PassedDuration <= ExpectedDuration;
    }

    CoroutineSuspensionPointsFlags ICoroutineSuspendor.SuspensionPoints
    {
        get => CoroutineSuspensionPointsFlags.Update;
    }
}
