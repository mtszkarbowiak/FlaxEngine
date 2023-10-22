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
    /// <summary>
    ///     Handle to a coroutine.
    /// </summary>
    public interface ICoroutine : IDisposable
    {
        /// <summary>
        ///     Attempts to make a step on the coroutine.
        /// </summary>
        /// <remarks>
        ///     Next element is taken from the coroutine's enumerator.
        /// </remarks>
        /// <returns>
        ///     <c>true</c> if the coroutine has not finished yet.
        /// </returns>
        bool Step(CoroutineSuspensionPointIndex suspensionPoint);
    }

    private sealed class CoroutineExecutor : ICoroutine
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

        /// <summary>
        ///     Terminates coroutine execution without waiting.
        /// </summary>
        /// <returns>
        ///     <c>true</c> if there are some skipped steps.
        /// </returns>
        public bool Stop()
        {
            _currentSuspendor = null;
            return _enumerator.MoveNext();
        }

        /// <inheritdoc/>
        public bool Step(CoroutineSuspensionPointIndex suspensionPoint)
        {
            //TODO(mtszkarbowiak) Implement.
        }
    }

    private readonly List<CoroutineExecutor> _executors = new();

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

    private void ExecuteStep(CoroutineSuspensionPointIndex suspensionPoint)
    {
        throw new NotImplementedException(); //TODO(mtszkarbowiak) Implement.
    }
}
