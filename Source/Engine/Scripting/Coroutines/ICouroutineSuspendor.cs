namespace FlaxEngine;

/// <summary>
///     Provides information about a suspension of a coroutine.
/// </summary>
/// <remarks>
///     This object will not be disposed automatically. It is up to the coroutine to dispose it.
/// </remarks>
public interface ICouroutineSuspendor //TODO(mtszkarbowiak) Fix typo.
{
    /// <summary>
    ///		Makes a single update coming from the executor.
    /// </summary>
    /// <returns>
    ///		<c>true</c> if the coroutine is still being blocker from further execution.
    /// </returns>
    bool Step(CoroutineSuspensionPointIndex suspensionPoint);

    /// <summary>
    ///		Set of suspension points used to check if the coroutine can be resumed.
    /// </summary>
    CoroutineSuspensionPointsFlags SuspensionPoints { get; }
}
