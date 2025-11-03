import gc
import datetime
import time


# Instance Counter metaclass with timestamps
class InstanceCounterMeta(type):
    def __new__(cls, name, bases, dct):
        print(f"Creating class: {name}")

        # Add cleanup method for instance deletion
        def __del__(self):
            name = getattr(self, "name", getattr(self, "value", "unknown"))
            print(f"   Deleting instance {self.instance_id}: {name}")
            self.__class__._instance_count -= 1

        dct["__del__"] = __del__
        return super().__new__(cls, name, bases, dct)

    def __init__(cls, name, bases, dct):
        super().__init__(name, bases, dct)
        cls._instance_count = 0
        cls._total_instances_created = 0
        cls._class_created_at_ms = int(datetime.datetime.now().timestamp() * 1000)

    def __call__(cls, *args, **kwargs):
        cls._instance_count += 1
        cls._total_instances_created += 1

        # Create instance manually to control initialization
        instance = cls.__new__(cls)
        instance.instance_id = cls._total_instances_created
        instance.created_at_ms = int(datetime.datetime.now().timestamp() * 1000)

        # Call __init__ with instance_id already set
        if isinstance(instance, cls):
            instance.__init__(*args, **kwargs)

        return instance


# Example class using the instance counter metaclass
class CountedClass_1(metaclass=InstanceCounterMeta):
    def __init__(self, name="default"):
        self.name = name

    def get_info(self):
        return f"Instance {self.instance_id}: {self.name} (created: {self.created_at_ms}ms)"

    @classmethod
    def get_instance_stats(cls):
        return {
            "active_instances": cls._instance_count,
            "total_created": cls._total_instances_created,
            "class_created_at_ms": cls._class_created_at_ms,
        }


# Second example class using the same metaclass
class CountedClass_2(metaclass=InstanceCounterMeta):
    def __init__(self, value=0):
        self.value = value

    def get_info(self):
        return f"Instance {self.instance_id}: value={self.value}"

    @classmethod
    def get_instance_stats(cls):
        return {
            "active_instances": cls._instance_count,
            "total_created": cls._total_instances_created,
        }


def test_instance_counter():
    """Test basic instance counting functionality."""
    print("1. Instance Counter:")
    obj1 = CountedClass_1("first")
    obj2 = CountedClass_1("second")
    obj3 = CountedClass_1("third")
    obj4 = CountedClass_1("fourth")

    print(f"   Created 4 instances")
    stats = CountedClass_1.get_instance_stats()
    print(f"   Stats: {stats}")

    # Test deletion
    print("   Deleting obj2 and obj3...")
    del obj2
    del obj3

    gc.collect()

    stats = CountedClass_1.get_instance_stats()
    print(f"   Stats after deletion: {stats}")

    return obj1, obj4


def test_multiple_classes():
    """Test that different classes have separate counters."""
    print()
    print("2. Multiple Classes:")

    counted = CountedClass_1("test")
    another = CountedClass_2(42)

    print(f"   CountedClass_1 stats: {CountedClass_1.get_instance_stats()}")
    print(f"   CountedClass_2 stats: {CountedClass_2.get_instance_stats()}")

    return counted, another


def test_timestamps():
    """Test timestamp functionality."""
    print()
    print("3. Timestamps:")
    time.sleep(0.1)  # Small delay
    obj = CountedClass_1("timestamp_test")

    print(f"   Instance created at: {obj.created_at_ms}ms")
    print(f"   Class created at: {CountedClass_1._class_created_at_ms}ms")

    return obj


def main():
    print("=== Instance Counter Metaclass Demo ===")

    # Test instance counting
    obj1, obj4 = test_instance_counter()

    # Test multiple classes
    counted, another = test_multiple_classes()

    # Test timestamps
    timestamp_obj = test_timestamps()

    # Cleanup
    del obj1, obj4, counted, another, timestamp_obj
    gc.collect()

    print()
    print("✅ Demo complete!")


if __name__ == "__main__":
    main()
