#nullable enable

namespace FlaxEngine;

/// <summary>
///		Suspends the coroutine until the specified predicate returns <c>true</c>. It suspends the coroutine at Update.
/// </summary>
/// <remarks>
///     Note: Make sure that the predicate captures the state of the coroutine correctly.
/// </remarks>
public sealed class WaitWhile : ICoroutineSuspender
{
    private readonly System.Func<bool> _predicate;

    /// <summary>
    ///     Initializes <see cref="WaitWhile"/>.
    /// </summary>
    public WaitWhile(System.Func<bool> predicate)
    {
        _predicate = predicate;
    }

    /// <inheritdoc />
    public bool Step(CoroutineSuspensionPointIndex suspensionPoint)
    {
        return _predicate();
    }

    /// <inheritdoc />
    public CoroutineSuspensionPointsFlags SuspensionPoints
    {
        get => CoroutineSuspensionPointsFlags.Update;
    }
}
