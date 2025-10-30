import asyncio


async def sample_async_function():
    print("Starting async function...")
    await asyncio.sleep(1)
    print("Async function completed!")


async def main():
    """Main coroutine to run the async function."""
    await sample_async_function()


if __name__ == "__main__":
    # Run the async main function
    asyncio.run(main())
