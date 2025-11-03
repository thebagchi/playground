import gc
import datetime


# Comprehensive metaclass demonstrating all possible hook points
class ComprehensiveMeta(type):
    def __new__(cls, name, bases, dct, **kwargs):
        """Called when the class is being created/defined."""
        print(f"__new__: Creating class '{name}' with bases {bases}")
        print(f"__new__: Class dictionary keys: {list(dct.keys())}")

        # Add instance tracking
        def __del__(self):
            print(f"__del__: Instance of {self.__class__.__name__} is being deleted")
            if hasattr(self.__class__, "_instances"):
                self.__class__._instances.discard(id(self))

        dct["__del__"] = __del__
        return super().__new__(cls, name, bases, dct, **kwargs)

    def __init__(cls, name, bases, dct, **kwargs):
        """Called after __new__ to initialize the newly created class."""
        print(f"__init__: Initializing class '{name}'")
        print(f"__init__: Bases: {bases}")
        super().__init__(name, bases, dct, **kwargs)

        # Initialize instance tracking
        cls._instances = set()
        cls._creation_time = datetime.datetime.now()

    @classmethod
    def __prepare__(cls, name, bases, **kwargs):
        """Called before class creation to prepare the namespace."""
        print(f"__prepare__: Preparing namespace for class '{name}'")

        # Return a custom dict that tracks attribute assignments
        class TrackingDict(dict):
            def __setitem__(self, key, value):
                print(f"__prepare__: Setting attribute '{key}' = {value}")
                super().__setitem__(key, value)

        return TrackingDict()

    def __call__(cls, *args, **kwargs):
        """Called when creating an instance of the class."""
        print(
            f"__call__: Creating instance of '{cls.__name__}' with args={args}, kwargs={kwargs}"
        )

        # Track the instance
        instance = super().__call__(*args, **kwargs)
        cls._instances.add(id(instance))

        print(f"__call__: Instance created: {instance}")
        print(f"__call__: Total instances of {cls.__name__}: {len(cls._instances)}")
        return instance

    def __init_subclass__(cls, **kwargs):
        """Called when a subclass of this metaclass's class is created."""
        print(f"__init_subclass__: New subclass '{cls.__name__}' created")
        print(f"__init_subclass__: kwargs: {kwargs}")
        super().__init_subclass__(**kwargs)

    def __subclasshook__(cls, subclass):
        """Called by isinstance() and issubclass() to customize behavior."""
        print(f"__subclasshook__: Checking if {subclass} is subclass of {cls}")
        return NotImplemented  # Let Python handle it normally

    def __instancecheck__(cls, instance):
        """Called by isinstance() to customize instance checking."""
        print(f"__instancecheck__: Checking if {instance} is instance of {cls}")
        return super().__instancecheck__(instance)

    def __subclasscheck__(cls, subclass):
        """Called by issubclass() to customize subclass checking."""
        print(f"__subclasscheck__: Checking if {subclass} is subclass of {cls}")
        return super().__subclasscheck__(subclass)


# Base class using the comprehensive metaclass
class BaseClass(metaclass=ComprehensiveMeta):
    """A base class that demonstrates metaclass hook points."""

    def __init__(self, name="default"):
        self.name = name
        print(f"BaseClass.__init__: Initialized with name='{name}'")

    def __init_subclass__(cls, **kwargs):
        """Called when a subclass is created."""
        print(
            f"BaseClass.__init_subclass__: New subclass '{cls.__name__}' created with kwargs={kwargs}"
        )
        super().__init_subclass__(**kwargs)

    def __repr__(self):
        return f"BaseClass(name='{self.name}')"


# Subclass to demonstrate __init_subclass__
class SubClass(BaseClass):
    """A subclass to trigger __init_subclass__ hook."""

    def __init__(self, name="sub_default", value=0):
        super().__init__(name)
        self.value = value
        print(f"SubClass.__init__: Initialized with value={value}")


def demonstrate_hooks():
    """Demonstrate all metaclass hook points."""
    print("\n" + "=" * 60)
    print("DEMONSTRATING ALL METACLASS HOOK POINTS")
    print("=" * 60)

    # 1. Class creation triggers: __prepare__, __new__, __init__
    print("\n1. CLASS CREATION (triggers __prepare__, __new__, __init__)")
    print("   Creating BaseClass...")

    # 2. Instance creation triggers: __call__
    print("\n2. INSTANCE CREATION (triggers __call__)")
    obj1 = BaseClass("first")
    obj2 = BaseClass("second")

    # 3. isinstance/issubclass checks trigger: __instancecheck__, __subclasscheck__, __subclasshook__
    print(
        "\n3. TYPE CHECKING (triggers __instancecheck__, __subclasscheck__, __subclasshook__)"
    )
    print(f"   isinstance(obj1, BaseClass): {isinstance(obj1, BaseClass)}")
    print(f"   issubclass(SubClass, BaseClass): {issubclass(SubClass, BaseClass)}")

    # 4. Subclass creation triggers: __init_subclass__
    print("\n4. SUBCLASS CREATION (triggers __init_subclass__)")
    print("   Creating SubClass...")
    sub_obj = SubClass("sub_instance", 42)

    # 5. Instance deletion triggers: __del__
    print("\n5. INSTANCE DELETION (triggers __del__)")
    print("   Deleting obj1...")
    del obj1

    print("   Deleting sub_obj...")
    del sub_obj

    # Force garbage collection
    gc.collect()

    print("   Deleting obj2...")
    del obj2
    gc.collect()

    print("\n" + "=" * 60)
    print("ALL HOOK POINTS DEMONSTRATED!")
    print("=" * 60)


if __name__ == "__main__":
    demonstrate_hooks()
