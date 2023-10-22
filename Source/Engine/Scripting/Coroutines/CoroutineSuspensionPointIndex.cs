namespace FlaxEngine;

/// <summary>
///     Identifier of a point in a game loop where a coroutine can execute.
/// </summary>
public enum CoroutineSuspensionPointIndex
{
    Update,
    LateUpdate,
    FixedUpdate,
    LateFixedUpdate,
}
