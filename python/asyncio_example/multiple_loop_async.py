import asyncio
import threading
import signal
import time
from queue import Queue
from typing import Any


async def producer_loop(message_queue: Queue, loop_name: str) -> None:
    print(f"[{loop_name}] Producer starting")
    for i in range(1, 11):
        message = f"Message {i} from {loop_name} producer"
        message_queue.put(message)
        print(f"[{loop_name}] Produced: {message}")
        await asyncio.sleep(0.5)
    message_queue.put(f"DONE_{loop_name}")
    print(f"[{loop_name}] Producer finished")


async def consumer_loop(
    message_queue: Queue, loop_name: str, shutdown_event: threading.Event
) -> None:
    print(f"[{loop_name}] Consumer starting")

    try:
        while not shutdown_event.is_set():
            if not message_queue.empty():
                message = message_queue.get()
                if message.startswith("DONE_"):
                    producer_loop = message.split("_")[1]
                    print(
                        f"[{loop_name}] Received completion signal from {producer_loop}"
                    )
                else:
                    print(f"[{loop_name}] Consumed: {message}")
                    await asyncio.sleep(0.2)
            else:
                await asyncio.sleep(0.1)
    except KeyboardInterrupt:
        print(f"[{loop_name}] Consumer interrupted by Ctrl+C")
    finally:
        print(f"[{loop_name}] Consumer finished - interrupted")


def run_loop_in_thread(loop: asyncio.AbstractEventLoop, coro, loop_name: str) -> None:
    print(f"Starting {loop_name} in thread: {threading.current_thread().name}")
    asyncio.set_event_loop(loop)
    try:
        loop.run_until_complete(coro)
    finally:
        loop.close()
    print(f"{loop_name} thread finished")


async def main() -> None:
    print("Multiple Event Loop Communication Demo")
    print("=" * 50)

    # Create a queue for communication
    message_queue = Queue()

    # Create shutdown event for graceful termination
    shutdown_event = threading.Event()

    # Set up signal handler for Ctrl+C
    def signal_handler(signum, frame):
        print("\nReceived Ctrl+C, signaling shutdown...")
        shutdown_event.set()

    signal.signal(signal.SIGINT, signal_handler)

    # Create two separate event loops
    loop1 = asyncio.new_event_loop()
    loop2 = asyncio.new_event_loop()

    # Create coroutines for each loop
    producer_coro = producer_loop(message_queue, "Loop1")
    consumer_coro = consumer_loop(message_queue, "Loop2", shutdown_event)

    # Create threads for each loop
    thread1 = threading.Thread(
        target=run_loop_in_thread,
        args=(loop1, producer_coro, "Loop1"),
        name="ProducerThread",
    )

    thread2 = threading.Thread(
        target=run_loop_in_thread,
        args=(loop2, consumer_coro, "Loop2"),
        name="ConsumerThread",
    )

    # Start both threads
    print("Starting producer and consumer threads...")
    thread1.start()
    thread2.start()

    # Wait for both threads to complete
    thread1.join()
    thread2.join()

    print("\nAll threads completed. Communication demo finished.")


if __name__ == "__main__":
    # Run the basic multiple loop communication demo
    asyncio.run(main())
