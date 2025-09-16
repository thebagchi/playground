import threading

class FizzBuzz:
    def __init__(self, n):
        self.n = n
        self.current = 1
        self.lock = threading.Lock()

    def fizz(self):
        while True:
            with self.lock:
                if self.current > self.n:
                    break
                if self.current % 3 == 0 and self.current % 5 != 0:
                    print(f"{self.current}: Fizz")
                    self.current += 1

    def buzz(self):
        while True:
            with self.lock:
                if self.current > self.n:
                    break
                if self.current % 5 == 0 and self.current % 3 != 0:
                    print(f"{self.current}: Buzz")
                    self.current += 1

    def fizzbuzz(self):
        while True:
            with self.lock:
                if self.current > self.n:
                    break
                if self.current % 3 == 0 and self.current % 5 == 0:
                    print(f"{self.current}: FizzBuzz")
                    self.current += 1

    def number(self):
        while True:
            with self.lock:
                if self.current > self.n:
                    break
                if self.current % 3 != 0 and self.current % 5 != 0:
                    print(f"{self.current}: {self.current}")
                    self.current += 1

def main():
    n = 15  # Run FizzBuzz up to 15
    fb = FizzBuzz(n)

    # Create threads for each function
    t1 = threading.Thread(target=fb.fizz)
    t2 = threading.Thread(target=fb.buzz)
    t3 = threading.Thread(target=fb.fizzbuzz)
    t4 = threading.Thread(target=fb.number)

    # Start all threads
    t1.start()
    t2.start()
    t3.start()
    t4.start()

    # Wait for all threads to complete
    t1.join()
    t2.join()
    t3.join()
    t4.join()

if __name__ == "__main__":
    main()