import asyncio


async def waiter(event):
    print("Waiting for event...")
    await event.wait()
    print("Event received!")


async def setter(event):
    await asyncio.sleep(2)
    print("Setting event")
    event.set()


async def main():
    event = asyncio.Event()
    await asyncio.gather(waiter(event), setter(event))


if __name__ == "__main__":
    # Run the async main function
    asyncio.run(main())
