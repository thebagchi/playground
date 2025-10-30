import asyncio
import random
from typing import Any, Awaitable, List


async def sample_task(name: str, delay: float) -> str:
    """A sample async task that simulates work with a delay."""
    print(f"Starting {name}")
    await asyncio.sleep(delay)
    print(f"Finished {name}")
    return f"{name} completed in {delay} seconds"


async def run_parallel(*coroutines: Awaitable[Any]) -> List[Any]:
    print(f"Running {len(coroutines)} tasks in parallel...")
    results = await asyncio.gather(*coroutines)
    print("All tasks completed!")
    return results


async def run_sequential(*coroutines: Awaitable[Any]) -> List[Any]:
    print(f"Running {len(coroutines)} tasks sequentially...")
    results = []
    for coroutine in coroutines:
        result = await coroutine
        results.append(result)
    print("All tasks completed!")
    return results


async def main() -> None:
    print("Sample 1: Calling individual tasks")
    print("-" * 40)
    # Create individual tasks
    task1 = sample_task("Task 1", 1.0)
    task2 = sample_task("Task 2", 2.0)
    task3 = sample_task("Task 3", 3.0)

    # Run them in parallel using the wrapper
    results = await run_parallel(task1, task2, task3)

    print("Results:")
    for result in results:
        print(f"- {result}")

    print("Sample 2: Using *tasks with many tasks")
    print("-" * 40)
    # Create 101 tasks with random delays between 0.1 and 3 seconds
    tasks = []
    for i in range(1, 102):  # 1 to 101 inclusive
        delay = random.uniform(0.1, 9.9)
        task = sample_task(f"Task {i}", delay)
        tasks.append(task)

    # Run them in parallel using the wrapper with *tasks
    results = await run_parallel(*tasks)

    print(f"--> Completed {len(results)} tasks. All results:")
    for result in results:
        print(f"- {result}")

    print("Sample 3: Sequential execution with individual tasks")
    print("-" * 40)
    # Create individual tasks for sequential demonstration (same timeouts as Sample 1)
    task1 = sample_task("SeqTask A", 1.0)
    task2 = sample_task("SeqTask B", 2.0)
    task3 = sample_task("SeqTask C", 3.0)

    # Run them sequentially using the wrapper with individual arguments
    results = await run_sequential(task1, task2, task3)

    print("Sequential results:")
    for result in results:
        print(f"- {result}")

    print("Sample 4: Using sequential execution")
    print("-" * 40)
    # Create a few tasks for sequential demonstration
    tasks = []
    for i in range(1, 102):  # 1 to 101 inclusive
        delay = random.uniform(0.1, 9.9)
        task = sample_task(f"Task {i}", delay)
        tasks.append(task)

    # Run them sequentially using the wrapper
    results = await run_sequential(*tasks)

    print(f"--> Completed {len(results)} tasks sequentially. All results:")
    for result in results:
        print(f"- {result}")


if __name__ == "__main__":
    # Run the async main function
    asyncio.run(main())
