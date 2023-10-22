#nullable enable

using FlaxEngine.Assertions;

namespace FlaxEngine;

/// <summary>
///     Suspends a coroutine until a certain amount of frames have passed. It suspends the coroutine at Update.
/// </summary>
/// <remarks>
///     To reduce GC pressure this class can be reused.
///     To do so, call <see cref="ExpectedFramesCount"/> and <see cref="PassedFrames"/> are exposed.
/// </remarks>
public sealed class WaitForFrames : ICoroutineSuspender
{
    /// <summary>
    ///		How many frames the coroutine should be suspended.
    /// </summary>
    public int PassedFrames { get; set; }

    /// <summary>
    ///		How many frames the coroutine has already been suspended.
    /// </summary>
    public int ExpectedFramesCount { get; set; }


    /// <summary>
    ///     Sets <see cref="WaitForFrames"/> up.
    /// </summary>
    public WaitForFrames(int expectedFramesCount = 0)
    {
        ExpectedFramesCount = expectedFramesCount;
        PassedFrames = 0;
    }

    /// <summary>
    ///     Resets the coroutine's suspension frames.
    /// </summary>
    public void Reset()
    {
        PassedFrames = 0;
    }


    bool ICoroutineSuspender.Step(CoroutineSuspensionPointIndex suspensionPoint)
    {
        Assert.IsTrue(
          suspensionPoint == CoroutineSuspensionPointIndex.Update,
          $"{nameof(WaitForFrames)} must not step at {suspensionPoint}."
        );

        PassedFrames++;
        return PassedFrames <= ExpectedFramesCount;
    }

    CoroutineSuspensionPointsFlags ICoroutineSuspender.SuspensionPoints
    {
        get => CoroutineSuspensionPointsFlags.Update;
    }
}
