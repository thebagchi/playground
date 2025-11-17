#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include "cmd.h"

int main(int argc, char* argv[]) {
  std::cout << "Hello World" << std::endl;

  // Example: Run the 'echo' command
  std::vector<std::string> echo_args = {"echo", "Hello from CMD class!"};
  CMD cmd(echo_args);

  if (cmd.execute()) {
    std::cout << "Command started with PID: " << cmd.get_pid() << std::endl;

    // Wait a bit for the command to execute
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Read output
    std::string output = cmd.read_stdout();
    if (!output.empty()) {
      std::cout << "Command output: " << output;
    }

    // Read any error output
    std::string error = cmd.read_stderr();
    if (!error.empty()) {
      std::cout << "Command error: " << error;
    }

    // Wait for command to complete
    int exit_code = cmd.wait();
    std::cout << "Command exited with code: " << exit_code << std::endl;
  } else {
    std::cout << "Failed to execute command" << std::endl;
  }

  return 0;
}