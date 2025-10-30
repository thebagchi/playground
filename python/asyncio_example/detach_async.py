import asyncio
from typing import Any, Awaitable


async def sample_task(name: str, delay: float) -> str:
    print(f"Starting {name}")
    await asyncio.sleep(delay)
    print(f"Finished {name}")
    return f"{name} completed in {delay} seconds"


async def detach_coroutine(
    coro: Awaitable[Any], task_name: str = "detached_task"
) -> None:
    task = asyncio.create_task(coro, name=task_name)
    print(f"Detached task '{task_name}' started in background")
    return None


async def main() -> None:
    print("Demonstrating detached coroutines")
    print("=" * 40)

    # Create some tasks
    task1 = sample_task("Background Task 1", 2.0)
    task2 = sample_task("Background Task 2", 3.0)
    task3 = sample_task("Background Task 3", 1.5)

    # Detach them - they will run in the background
    print("Detaching tasks...")
    await detach_coroutine(task1, "bg_task_1")
    await detach_coroutine(task2, "bg_task_2")
    await detach_coroutine(task3, "bg_task_3")

    # Do some other work while tasks run in background
    print("Doing other work while tasks run in background...")
    await asyncio.sleep(0.5)
    print("Still working...")

    # Wait a bit more to let background tasks complete
    print("Waiting for background tasks to complete...")
    await asyncio.sleep(3.5)  # Wait longer than the longest task

    print("All background tasks should be completed now.")


if __name__ == "__main__":
    # Run the basic detach demonstration
    asyncio.run(main())
