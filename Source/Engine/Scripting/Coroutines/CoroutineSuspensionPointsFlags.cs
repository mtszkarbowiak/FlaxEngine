namespace FlaxEngine;

/// <summary>
///     Set of points in a game loop where a coroutine can execute.
/// </summary>
[System.Flags]
public enum CoroutineSuspensionPointsFlags : uint
{
    None = 0,

    Update = 1 << CoroutineSuspensionPointIndex.Update,
    LateUpdate = 1 << CoroutineSuspensionPointIndex.LateUpdate,
    FixedUpdate = 1 << CoroutineSuspensionPointIndex.FixedUpdate,
    LateFixedUpdate = 1 << CoroutineSuspensionPointIndex.LateFixedUpdate,
}
