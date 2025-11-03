import gc


# Custom metaclass
class MetaClass(type):
    def __new__(cls, name, bases, dct):
        # Called when a class using this metaclass is being created/defined
        # This happens at class definition time, not instance creation time
        print("inside __new__ in MetaClass ...")

        # Add a __del__ method to track instance deletion
        def __del__(self):
            print(f"Instance of {self.__class__.__name__} is being deleted: {self}")

        dct["__del__"] = __del__
        return super().__new__(cls, name, bases, dct)

    def __init__(cls, name, bases, dct):
        # Called after __new__ to initialize the newly created class
        # This happens immediately after class creation at definition time
        print("inside __init__ in MetaClass ...")
        super().__init__(name, bases, dct)

    def __call__(cls, *args, **kwargs):
        # Called when creating an instance of the class (e.g., DataClass())
        # This happens at instance creation time, not class definition time
        print("inside __call__ in MetaClass ...")
        return super().__call__(*args, **kwargs)


# Use it
class DataClass(metaclass=MetaClass):
    pass


def main():
    print("main function starts ...")

    # Create an instance of DataClass to trigger __call__ in metaclass
    instance = DataClass()
    print(f"DataClass instance created: {instance}")

    # Explicitly delete the instance to trigger __del__
    print("deleting the instance...")
    del instance

    # Force garbage collection to ensure __del__ is called
    gc.collect()
    print("Instance deleted and garbage collected")

    print("main function ends ...")


if __name__ == "__main__":
    main()
