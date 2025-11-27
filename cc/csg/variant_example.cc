#include <netinet/in.h>  // For sockaddr_in, sockaddr_in6

#include <iostream>
#include <variant>

using SockAddr4 = struct sockaddr_in;
using SockAddr6 = struct sockaddr_in6;
using SockAddr = std::variant<SockAddr4, SockAddr6>;

// Using std::visit with constexpr for compile-time dispatch
void print_address_visit(const SockAddr& addr) {
  std::visit(
      [](const auto& sa) {
        if constexpr (std::is_same_v<std::decay_t<decltype(sa)>, SockAddr4>) {
          std::cout << "IPv4 address (visit)\n";
        } else if constexpr (std::is_same_v<std::decay_t<decltype(sa)>,
                                            SockAddr6>) {
          std::cout << "IPv6 address (visit)\n";
        }
      },
      addr);
}

// Using std::holds_alternative for runtime type checking
void print_address_holds(const SockAddr* addr) {
  if (std::holds_alternative<SockAddr4>(*addr)) {
    std::cout << "IPv4 address (holds)\n";
  } else if (std::holds_alternative<SockAddr6>(*addr)) {
    std::cout << "IPv6 address (holds)\n";
  }
}

int main() {
  SockAddr addr4 = SockAddr4{};
  SockAddr addr6 = SockAddr6{};

  print_address_visit(addr4);  // Compile-time dispatch
  print_address_visit(addr6);

  print_address_holds(&addr4);  // Runtime checking
  print_address_holds(&addr6);

  return 0;
}