import time
import asyncio
import inspect


def time_taken(func):
    if inspect.iscoroutinefunction(func):

        async def async_wrapper(*args, **kwargs):
            start_time = time.perf_counter()
            result = await func(*args, **kwargs)
            end_time = time.perf_counter()
            elapsed_time = end_time - start_time
            print(f"Function {func.__name__} took {elapsed_time:.2f} seconds")
            return result

        return async_wrapper
    else:

        def sync_wrapper(*args, **kwargs):
            start_time = time.perf_counter()
            result = func(*args, **kwargs)
            end_time = time.perf_counter()
            elapsed_time = end_time - start_time
            print(f"Function {func.__name__} took {elapsed_time:.2f} seconds")
            return result

        return sync_wrapper


# Test the decorator
@time_taken
def example_function(n):
    time.sleep(0.1)  # Simulate some work
    return sum(range(n))


@time_taken
async def async_example_function(n):
    await asyncio.sleep(0.1)  # Simulate some async work
    return sum(range(n))


if __name__ == "__main__":
    # Test sync function
    result = example_function(100000)
    print(f"Sync Result: {result}")

    # Test async function
    async_result = asyncio.run(async_example_function(100000))
    print(f"Async Result: {async_result}")
