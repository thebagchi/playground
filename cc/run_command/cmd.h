#ifndef CMD_H_INCLUDED
#define CMD_H_INCLUDED

#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <vector>

class CMD {
private:
  std::vector<std::string> args_;
  pid_t pid_;
  int stdi_[2]; // For writing to child
  int stdo_[2]; // For reading from child
  int stde_[2]; // For reading stderr from child
  bool running_;
  std::string dir_;
  std::map<std::string, std::string> env_;

  void setup_child_pipes() {
    // Close unused pipe ends
    close(stdi_[1]); // Close write end of stdin pipe
    close(stdo_[0]); // Close read end of stdout pipe
    close(stde_[0]); // Close read end of stderr pipe

    // Redirect stdin, stdout, stderr
    dup2(stdi_[0], STDIN_FILENO);
    dup2(stdo_[1], STDOUT_FILENO);
    dup2(stde_[1], STDERR_FILENO);

    // Close the duplicated pipe ends
    close(stdi_[0]);
    close(stdo_[1]);
    close(stde_[1]);
  }

  void setup_parent_pipes() {
    // Close unused pipe ends in parent
    close(stdi_[0]); // Close read end of stdin pipe
    close(stdo_[1]); // Close write end of stdout pipe
    close(stde_[1]); // Close write end of stderr pipe

    // Set pipes to non-blocking mode for reading
    fcntl(stdo_[0], F_SETFL, O_NONBLOCK);
    fcntl(stde_[0], F_SETFL, O_NONBLOCK);
  }
public:
  CMD(const std::vector<std::string>& args)
      : args_(args), pid_(-1), running_(false), dir_(""), env_() {
    // Constructor
    // Initialize pipe arrays
    stdi_[0] = stdi_[1] = -1;
    stdo_[0] = stdo_[1] = -1;
    stde_[0] = stde_[1] = -1;
  }

  ~CMD() {
    // Destructor
    // Close any open pipes
    if (stdi_[0] != -1) {
      close(stdi_[0]);
    }
    if (stdi_[1] != -1) {
      close(stdi_[1]);
    }
    if (stdo_[0] != -1) {
      close(stdo_[0]);
    }
    if (stdo_[1] != -1) {
      close(stdo_[1]);
    }
    if (stde_[0] != -1) {
      close(stde_[0]);
    }
    if (stde_[1] != -1) {
      close(stde_[1]);
    }

    // If still running, terminate the process
    if (running_ && pid_ > 0) {
      kill(pid_, SIGTERM);
      waitpid(pid_, nullptr, 0);
    }
  }

  bool execute() {
    // Execute the command
    // Create pipes for stdin, stdout, stderr
    if (pipe(stdi_) == -1 || pipe(stdo_) == -1 || pipe(stde_) == -1) {
      std::cerr << "Failed to create pipes" << std::endl;
      return false;
    }

    // Fork the process
    pid_ = fork();
    if (pid_ == -1) {
      std::cerr << "Failed to fork" << std::endl;
      return false;
    }

    if (pid_ == 0) { // Child process
      setup_child_pipes();

      // Change working directory if set
      if (!dir_.empty()) {
        if (chdir(dir_.c_str()) == -1) {
          _exit(1);
        }
      }

      // Set environment variables if any
      for (const auto& env_pair : env_) {
        setenv(env_pair.first.c_str(), env_pair.second.c_str(), 1);
      }

      // Prepare arguments for execvp
      std::vector<char*> argv;
      for (const auto& arg : args_) {
        argv.push_back(const_cast<char*>(arg.c_str()));
      }
      argv.push_back(nullptr);

      // Execute the command
      execvp(argv[0], argv.data());

      // If we reach here, execvp failed
      std::cerr << "Failed to execute command: " << args_[0] << std::endl;
      _exit(1);
    } else { // Parent process
      running_ = true;

      setup_parent_pipes();

      return true;
    }
  }

  bool write(const std::string& data) {
    // Write data to the command's stdin
    if (!running_ || stdi_[1] == -1) {
      return false;
    }

    ssize_t written = ::write(stdi_[1], data.c_str(), data.size());
    return written == static_cast<ssize_t>(data.size());
  }

  std::string read_stdout() {
    // Read data from the command's stdout
    if (!running_ || stdo_[0] == -1) {
      return "";
    }

    std::string buffer;
    buffer.reserve(4096);
    ssize_t bytes_read = read(stdo_[0], buffer.data(), buffer.capacity() - 1);

    if (bytes_read > 0) {
      buffer.resize(bytes_read);
      return buffer;
    }

    return "";
  }

  std::string read_stderr() {
    // Read data from the command's stderr
    if (!running_ || stde_[0] == -1) {
      return "";
    }

    std::string buffer;
    buffer.reserve(4096);
    ssize_t bytes_read = read(stde_[0], buffer.data(), buffer.capacity() - 1);

    if (bytes_read > 0) {
      buffer.resize(bytes_read);
      return buffer;
    }

    return "";
  }

  int wait() {
    // Wait for the command to complete
    if (!running_ || pid_ == -1) {
      return -1;
    }

    int status;
    if (waitpid(pid_, &status, 0) == -1) {
      return -1;
    }

    running_ = false;

    // Close remaining pipe ends
    if (stdi_[1] != -1) {
      close(stdi_[1]);
      stdi_[1] = -1;
    }
    if (stdo_[0] != -1) {
      close(stdo_[0]);
      stdo_[0] = -1;
    }
    if (stde_[0] != -1) {
      close(stde_[0]);
      stde_[0] = -1;
    }

    if (WIFEXITED(status)) {
      return WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
      return -WTERMSIG(status);
    }

    return -1;
  }

  bool is_running() const {
    // Check if command is still running
    return running_;
  }

  pid_t get_pid() const {
    // Get the process ID
    return pid_;
  }

  void send_signal(int signal) {
    // Send a signal to the child process
    if (running_ && pid_ > 0) {
      ::kill(pid_, signal);
    }
  }

  void terminate() {
    // Gracefully terminate the child process
    send_signal(SIGTERM);
  }

  void force_kill() {
    // Forcefully terminate the child process
    send_signal(SIGKILL);
  }

  int get_exit_status() {
    // Get the exit status without waiting
    if (!running_ || pid_ == -1) {
      return -1;
    }

    int status;
    pid_t result = waitpid(pid_, &status, WNOHANG);

    if (result == pid_) {
      running_ = false;

      // Close remaining pipe ends
      if (stdi_[1] != -1) {
        close(stdi_[1]);
        stdi_[1] = -1;
      }
      if (stdo_[0] != -1) {
        close(stdo_[0]);
        stdo_[0] = -1;
      }
      if (stde_[0] != -1) {
        close(stde_[0]);
        stde_[0] = -1;
      }

      if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
      } else if (WIFSIGNALED(status)) {
        return -WTERMSIG(status);
      }
    } else if (result == 0) {
      return -2; // Still running
    }

    return -1;
  }

  void detach() {
    // Detach the child process so it runs independently
    if (running_) {
      // Close all pipes
      if (stdi_[0] != -1) {
        close(stdi_[0]);
      }
      if (stdi_[1] != -1) {
        close(stdi_[1]);
      }
      if (stdo_[0] != -1) {
        close(stdo_[0]);
      }
      if (stdo_[1] != -1) {
        close(stdo_[1]);
      }
      if (stde_[0] != -1) {
        close(stde_[0]);
      }
      if (stde_[1] != -1) {
        close(stde_[1]);
      }

      // Reset state so parent no longer manages the process
      running_ = false;
      pid_ = -1;
    }
  }

  void set_working_directory(const std::string& path) {
    // Set the working directory for the child process
    dir_ = path;
  }

  void set_environment(const std::map<std::string, std::string>& env) {
    // Set environment variables for the child process
    env_ = env;
  }

  int get_stdin() const {
    // Get the stdin file descriptor
    return running_ ? stdi_[1] : -1;
  }
  int get_stdout() const {
    // Get the stdout file descriptor
    return running_ ? stdo_[0] : -1;
  }
  int get_stderr() const {
    // Get the stderr file descriptor
    return running_ ? stde_[0] : -1;
  }
};

#endif // CMD_H_INCLUDED