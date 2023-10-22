using System;

namespace FlaxEngine;

using System.Collections.Generic;

/// <summary>
///     Script that can controls execution of coroutines.
/// </summary>
/// <remarks>
///     Coroutines are executed in order of <see cref="Script"/>s execution, followed by <see cref="StartCoroutine"/> invocation.
///     If you want to stop a coroutine before it finishes, you can use <see cref="StopCoroutine"/>.
/// </remarks>
public class CoroutinesScript : Script
{
    public interface ICoroutine
    {
        //TODO(mtszkarbowiak) Implement.
    }

    public ICoroutine StartCoroutine(IEnumerator<ICouroutineSuspendor> routine)
    {
        throw new NotImplementedException(); //TODO(mtszkarbowiak) Implement.
    }

    public void StopCoroutine(ICoroutine routine)
    {
        throw new NotImplementedException(); //TODO(mtszkarbowiak) Implement.
    }

    public void StopAllCoroutines()
    {
        throw new NotImplementedException(); //TODO(mtszkarbowiak) Implement.
    }

    private void ExecuteStep()
    {
        throw new NotImplementedException(); //TODO(mtszkarbowiak) Implement.
    }
}
