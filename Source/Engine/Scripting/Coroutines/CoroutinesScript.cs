#nullable enable

using System;
using System.Linq;
using System.Collections.Generic;

namespace FlaxEngine;

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
        private readonly IEnumerator<ICoroutineSuspendor> _enumerator;
        private ICoroutineSuspendor? _currentSuspendor;

        public CoroutineExecutor(CoroutinesScript origin, IEnumerator<ICoroutineSuspendor> enumerator)
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
            var suspensionPointAsFlag = (CoroutineSuspensionPointsFlags)(1 << (int)suspensionPoint);

            // If there is no blocker ahead of the current coroutine, get to the next one.
            if (_currentSuspendor == null)
            {
                // If there is a coroutine to execute, execute it.
                if (_enumerator.MoveNext()) // Code between blocking points is invoked here.
                {
                    _currentSuspendor = _enumerator.Current;
                }

                // If there is no more coroutine code to execute, return false.
                else
                {
                    return false;
                }
            }

            // If the current coroutine does not suspend at the given point, return true and keep coroutine suspended at given blocker.
            if (!_currentSuspendor.SuspensionPoints.HasFlag(suspensionPointAsFlag))
            {
                return true;
            }

            // Update the current coroutine blocker.
            var blocking = _currentSuspendor.Step(suspensionPoint);

            // If the current coroutine blocker is not blocking anymore, set it to null and prepare to execute the next chunk of coroutine.
            if (!blocking)
            {
                _currentSuspendor = null;
            }

            return true;
        }
    }

    private readonly List<CoroutineExecutor> _executors = new();

    /// <summary>
    ///     Starts a new coroutine.
    /// </summary>
    /// <param name="routine">
    ///     Enumerator function to have its code executed.
    /// </param>
    /// <returns>
    ///     Handle to the coroutine.
    /// </returns>
    public ICoroutine StartCoroutine(IEnumerator<ICoroutineSuspendor> routine)
    {
        var executor = new CoroutineExecutor(this, routine);
        _executors.Add(executor);
        return executor;
    }

    /// <summary>
    ///     Stops coroutine before reaching its end. Removes link between the coroutine and its origin script.
    /// </summary>
    /// <remarks>
    ///     Warning: Coroutine enumerator function can have logic of disposal of object at its end. It may be skipped.
    /// </remarks>
    /// <exception cref="InvalidOperationException">
    ///     The coroutine must be terminated by the same script which created it.
    /// </exception>
    public bool StopCoroutine(ref ICoroutine? coroutine)
    {
        if (coroutine is not CoroutineExecutor executor)
        {
            throw new InvalidOperationException("This is not a coroutine coming from this script!");
        }

        var result = executor.Stop();
        executor.Dispose();

        if (!_executors.Remove(executor))
        {
            throw new InvalidOperationException("You must not stop a coroutine created by a different script!");
        }

        coroutine = null;
        return result;
    }

    /// <summary>
    ///     Stops all coroutines (using <see cref="StopCoroutine"/>).
    /// </summary>
    public void StopAllCoroutines()
    {
        while (_executors.Count > 0)
        {
            ICoroutine? executor = _executors.Last();
            StopCoroutine(ref executor);
        }
    }

    /// <summary>
    ///     Disposes all coroutines (using <see cref="ICoroutine.Dispose"/>) without stopping them.
    /// </summary>
    protected void TerminateAllCoroutines()
    {
        foreach (var executor in _executors)
        {
            executor.Dispose();
        }

        _executors.Clear();
    }


    private void ExecuteStep(CoroutineSuspensionPointIndex suspensionPoint)
    {
        for (var i = 0; i < _executors.Count; i++)
        {
            var executor = _executors[i];
            if (executor.Step(suspensionPoint))
            {
                continue;
            }
            
            ICoroutine? stoppedCoroutine = executor;
            StopCoroutine(ref stoppedCoroutine);
            i--;
        }
    }


    /// <inheritdoc />
    public override void OnUpdate()
    {
        base.OnUpdate();

        ExecuteStep(CoroutineSuspensionPointIndex.Update);
    }

    /// <inheritdoc />
    public override void OnFixedUpdate()
    {
        base.OnFixedUpdate();

        ExecuteStep(CoroutineSuspensionPointIndex.FixedUpdate);
    }

    /// <inheritdoc />
    public override void OnLateUpdate()
    {
        base.OnLateUpdate();

        ExecuteStep(CoroutineSuspensionPointIndex.LateUpdate);
    }

    /// <inheritdoc />
    public override void OnLateFixedUpdate()
    {
        base.OnLateFixedUpdate();

        ExecuteStep(CoroutineSuspensionPointIndex.LateFixedUpdate);
    }


    /// <inheritdoc />
    public override void OnDestroy()
    {
        base.OnDestroy();

        TerminateAllCoroutines();
    }
}
