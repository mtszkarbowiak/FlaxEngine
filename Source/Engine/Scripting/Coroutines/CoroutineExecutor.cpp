// Copyright (c) 2012-2024 Wojciech Figat. All rights reserved.

#include "CoroutineExecutor.h"

#include "Engine/Debug/DebugLog.h"
#include "Engine/Profiler/ProfilerCPU.h"

ScriptingObjectReference<CoroutineHandle> CoroutineExecutor::ExecuteOnce(
    ScriptingObjectReference<CoroutineBuilder> builder,
    const CoroutineSuspendPoint accumulationPoint
)
{
    const ExecutionID id = _uuidGenerator.Generate();
    Execution execution{ MoveTemp(builder), accumulationPoint, id };
    execution.ContinueCoroutine(CoroutineSuspendPoint::Update, Delta{ 0.0f, 0 });
    _executions.Add(MoveTemp(execution));

    ScriptingObjectReference<CoroutineHandle> handle = NewObject<CoroutineHandle>();
    handle->ExecutionID = id;
    handle->Executor = ScriptingObjectReference<CoroutineExecutor>{ this };
    return handle;
}

ScriptingObjectReference<CoroutineHandle> CoroutineExecutor::ExecuteRepeats(
    ScriptingObjectReference<CoroutineBuilder> builder,
    const CoroutineSuspendPoint accumulationPoint,
    const int32 repeats
)
{
    if (repeats <= 0) 
    {
        DebugLog::LogError(String::Format(
            TEXT("Coroutine must not be dispatched non-positive number of times! Call to repeat {} times will be ignored."), 
            repeats
        ));
        return nullptr;
    }

    const ExecutionID id = _uuidGenerator.Generate();
    Execution execution{ MoveTemp(builder), accumulationPoint, id, repeats };
    execution.ContinueCoroutine(CoroutineSuspendPoint::Update, Delta{ 0.0f, 0 });
    _executions.Add(MoveTemp(execution));

    ScriptingObjectReference<CoroutineHandle> handle = NewObject<CoroutineHandle>();
    handle->ExecutionID = id;
    handle->Executor = ScriptingObjectReference<CoroutineExecutor>{ this };
    return handle;
}

ScriptingObjectReference<CoroutineHandle> CoroutineExecutor::ExecuteLooped(
    ScriptingObjectReference<CoroutineBuilder> builder,
    const CoroutineSuspendPoint accumulationPoint
)
{
    const ExecutionID id = _uuidGenerator.Generate();
    Execution execution{ MoveTemp(builder), accumulationPoint, id, Execution::InfiniteRepeats };
    execution.ContinueCoroutine(CoroutineSuspendPoint::Update, Delta{ 0.0f, 0 });
    _executions.Add(MoveTemp(execution));

    ScriptingObjectReference<CoroutineHandle> handle = NewObject<CoroutineHandle>();
    handle->ExecutionID = id;
    handle->Executor = ScriptingObjectReference<CoroutineExecutor>{ this };
    return handle;
}


void CoroutineExecutor::Continue(
    const CoroutineSuspendPoint point,
    const int32 frames,
    const float deltaTime
)
{
    PROFILE_CPU();

    const Delta delta{ deltaTime, frames };

    for (int32 i = 0; i < _executions.Count();)
    {
        Execution& execution = _executions[i];
        const bool reachedEnd = execution.ContinueCoroutine(point, delta);

        if (reachedEnd)
            _executions.RemoveAt(i);
        else
            i++; // Increment the index only if the coroutine was not removed.
    }
}

int32 CoroutineExecutor::GetCoroutinesCount() const
{
    return _executions.Count();
}

using Step     = CoroutineBuilder::Step;
using StepType = CoroutineBuilder::StepType;

CoroutineExecutor::Execution::Execution(
    BuilderReference&& builder,
    const SuspendPoint accumulationPoint,
    const ExecutionID id,
    const int32 repeats
)   : _builder{ MoveTemp(builder) }
    , _accumulator{ 0.0f, 0 }
    , _id{ id }
    , _stepIndex{ 0 }
    , _repeats{ repeats }
    , _accumulationPoint{ accumulationPoint }
    , _isPaused{ false }
{
}

bool CoroutineExecutor::Execution::ContinueCoroutine(
    const CoroutineSuspendPoint point, 
    const Delta& delta
)
{
    if (_isPaused)
        return false;

    Delta deltaCopy = delta;

    ASSERT(_builder->GetSteps().Count() > 0); // Coroutines must have at least one step.
    ASSERT(_repeats != 0); // Coroutines must have at least one repeat.

    while (_repeats > 0 || _repeats == InfiniteRepeats)
    {
        const Array<Step>& steps = _builder->GetSteps();
        while (_stepIndex < steps.Count())
        {
            const Step& step = steps[_stepIndex];
            const bool isAccumulating = point == _accumulationPoint;

            if (!TryMakeStep(step, point, isAccumulating, deltaCopy, this->_accumulator))
                return false; // The coroutine is waiting for the next frame or seconds.

            ++_stepIndex;
        }

        _stepIndex = 0;

        if (_repeats != InfiniteRepeats)
            --_repeats;
    }

    return true; // The coroutine reached the end of the steps.
}

CoroutineExecutor::ExecutionID CoroutineExecutor::Execution::GetID() const
{
    return _id;
}

bool CoroutineExecutor::Execution::IsPaused() const
{
    return _isPaused;
}

void CoroutineExecutor::Execution::SetPaused(const bool value)
{
    _isPaused = value;
}

bool CoroutineExecutor::Execution::TryMakeStep(
    const CoroutineBuilder::Step& step, 
    const CoroutineSuspendPoint point,
    const bool isAccumulating,
    Delta& delta,
    Delta& accumulator
)
{
    switch (step.GetType())
    {
        case StepType::Run:
        {
            step.GetRunnable()->OnRun();
            return true;
        }

        case StepType::WaitSuspensionPoint:
        {
            return step.GetSuspensionPoint() == point;
        }

        case StepType::WaitSeconds:
        {
            if (!isAccumulating)
                return false;

            accumulator.time += delta.time;        // Transfer delta time to the accumulator.
            delta = Delta{ 0.0f, 0 }; // Reset the delta time after transferring it to the accumulator.

            const float secondsDelay = step.GetSecondsDelay();
            if (secondsDelay > accumulator.time)
                return false;

            accumulator.time -= secondsDelay;
            return true;
        }

        case StepType::WaitFrames:
        {
            if (!isAccumulating)
                return false;

            accumulator.frames += delta.frames;    // Transfer delta frames to the accumulator.
            delta = Delta{ 0.0f, 0 }; // Reset the delta frames after transferring it to the accumulator.

            const int32 framesDelay = step.GetFramesDelay();
            if (framesDelay > accumulator.frames)
                return false;

            accumulator.frames -= framesDelay;
            return true;
        }

        case StepType::WaitUntil:
        {
            bool result = false;
            step.GetPredicate()->OnCheck(result);
            return result;
        }

        case StepType::None:
        default: CRASH;

        //TODO(mtszkarbowiak) Optimize filtering accumulation steps by caching the expected suspend point. (Or filtering it by bit-field)
    }
}

bool CoroutineExecutor::HasFinished(const CoroutineHandle& handle) const
{
    PROFILE_CPU();

    for (const Execution& execution : _executions)
    {
        if (execution.GetID() == handle.ExecutionID)
            return false;
    }

    return true;
}

bool CoroutineExecutor::IsPaused(const CoroutineHandle& handle) const
{
    PROFILE_CPU();

    for (const Execution& execution : _executions)
    {
        if (execution.GetID() == handle.ExecutionID)
            return execution.IsPaused();
    }

    return false;
}

// Cancel, Pause and Resume currently have O(n) based on the number of coroutines.
// Subject to change, if the number of coroutines becomes a bottleneck.

bool CoroutineExecutor::Cancel(CoroutineHandle& handle)
{
    PROFILE_CPU();

    for (int32 i = 0; i < _executions.Count(); i++)
    {
        if (_executions.Get()[i].GetID() != handle.ExecutionID)
            continue;

        _executions.RemoveAt(i);
        handle.Executor = nullptr; // Nullify the reference to remove circular dependency.

        return true;
    }

    return false;
}

bool CoroutineExecutor::Pause(CoroutineHandle& handle)
{
    PROFILE_CPU();

    for (int32 i = 0; i < _executions.Count(); i++)
    {
        Execution& execution = _executions.Get()[i];

        if (execution.GetID() != handle.ExecutionID)
            continue;

        const bool wasPaused = execution.IsPaused();
        execution.SetPaused(true);
        return !wasPaused;
    }

    return false;
}

bool CoroutineExecutor::Resume(CoroutineHandle& handle)
{
    PROFILE_CPU();

    for (int32 i = 0; i < _executions.Count(); i++)
    {
        Execution& execution = _executions.Get()[i];

        if (execution.GetID() != handle.ExecutionID)
            continue;

        const bool wasPaused = execution.IsPaused();
        execution.SetPaused(false);
        return wasPaused;
    }

    return false;
};
