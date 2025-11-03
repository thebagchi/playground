import gc
import threading


# Thread-safe Singleton metaclass
class SingletonMeta(type):
    def __new__(cls, name, bases, dct):
        print(f"Creating class: {name}")

        # Add cleanup method for instance deletion
        def __del__(self):
            with self.__class__._lock:
                self.__class__._instance = None

        dct["__del__"] = __del__
        return super().__new__(cls, name, bases, dct)

    def __init__(cls, name, bases, dct):
        super().__init__(name, bases, dct)
        cls._instance = None
        cls._lock = threading.Lock()

    def __call__(cls, *args, **kwargs):
        # Double-checked locking for thread safety
        if cls._instance is None:
            with cls._lock:
                if cls._instance is None:
                    cls._instance = super().__call__(*args, **kwargs)
        return cls._instance


# Example singleton class
class Singleton(metaclass=SingletonMeta):
    def __init__(self, value="default"):
        if not hasattr(self, "_initialized"):
            self.value = value
            self._initialized = True

    def get_value(self):
        return self.value


def test_basic_singleton():
    """Test basic singleton behavior without threads."""
    print("1. Basic Singleton:")
    s1 = Singleton("first")
    s2 = Singleton("second")  # Should return same instance
    print(f"   s1 address: {hex(id(s1))}")
    print(f"   s2 address: {hex(id(s2))}")
    print(f"   s1 is s2: {s1 is s2}")  # True
    print(f"   Value: {s1.get_value()}")  # "first" (first initialization wins)
    return s1, s2


def test_thread_safety():
    """Test singleton thread safety with multiple concurrent threads."""
    print("\n2. Thread Safety:")
    results = []

    def create_singleton(thread_id):
        instance = Singleton(f"thread_{thread_id}")
        results.append((thread_id, id(instance)))
        print(f"   Thread {thread_id}: address {hex(id(instance))}")

    threads = [threading.Thread(target=create_singleton, args=(i,)) for i in range(3)]
    for t in threads:
        t.start()
    for t in threads:
        t.join()

    unique_ids = set(result[1] for result in results)
    print(f"   Unique instance IDs: {len(unique_ids)} (should be 1)")
    return len(unique_ids) == 1


def test_cleanup_and_recreation():
    """Test singleton cleanup and recreation after deletion."""
    print("\n3. Cleanup and Recreation:")
    # Clean up any existing instances
    gc.collect()

    s = Singleton("after_cleanup")
    print(f"   New instance address: {hex(id(s))}")
    print(f"   New instance value: {s.get_value()}")
    return s


def main():
    print("=== Singleton Pattern Demo ===")

    # Test basic singleton behavior
    s1, s2 = test_basic_singleton()

    # Test thread safety
    thread_safe = test_thread_safety()

    # Test cleanup and recreation
    s3 = test_cleanup_and_recreation()

    # Clean up
    del s1, s2, s3
    gc.collect()

    status = "✅ PASSED" if thread_safe else "❌ FAILED"
    print(f"\n{status} - Demo complete!")


if __name__ == "__main__":
    main()
