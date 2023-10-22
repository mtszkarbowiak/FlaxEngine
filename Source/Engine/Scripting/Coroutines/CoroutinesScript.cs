#nullable enable

using System;
using System.Linq;

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

    private sealed class CoroutineExecutor : ICoroutine, IDisposable
    {
        private readonly CoroutinesScript _origin;
        private readonly IEnumerator<ICouroutineSuspendor> _enumerator;
        private ICouroutineSuspendor? _currentSuspendor;

        public CoroutineExecutor(CoroutinesScript origin, IEnumerator<ICouroutineSuspendor> enumerator)
        {
            _origin = origin;
            _enumerator = enumerator;
            _currentSuspendor = _enumerator.Current;
        }

        /// <inheritdoc />
        public void Dispose()
        {
            if (_currentSuspendor != null)
            {
                Debug.LogWarning(
                    $"The coroutine has not ended its execution before disposal. " +
                    $"Suspendor: {_currentSuspendor} (on {_currentSuspendor?.SuspensionPoints})"
                );
            }

            _enumerator.Dispose();
        }

        public void Stop()
        {
            //TODO(mtszkarbowiak) Implement.
        }

        public void Execute(CoroutineSuspensionPointIndex suspensionPoint)
        {
            //TODO(mtszkarbowiak) Implement.
        }
    }

    private List<CoroutineExecutor> _executors = new();

    public ICoroutine StartCoroutine(IEnumerator<ICouroutineSuspendor> routine)
    {
        var executor = new CoroutineExecutor(this, routine);
        _executors.Add(executor);
        return executor;
    }

    public void StopCoroutine(ref ICoroutine? coroutine)
    {
        if (coroutine is not CoroutineExecutor executor)
        {
            throw new InvalidOperationException("This is not a coroutine coming from this script!");
        }

        executor.Stop();
        executor.Dispose();

        if (!_executors.Remove(executor))
        {
            throw new InvalidOperationException("You must not stop a coroutine created by a different script!");
        }

        coroutine = null;
    }

    public void StopAllCoroutines()
    {
        while (_executors.Count > 0)
        {
            ICoroutine? executor = _executors.Last();
            StopCoroutine(ref executor);
        }
    }

    private void ExecuteStep()
    {
        throw new NotImplementedException(); //TODO(mtszkarbowiak) Implement.
    }
}
