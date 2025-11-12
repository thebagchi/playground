#include <iostream>

template <typename... T>
constexpr void count_params(T... args) {
    static_assert(sizeof...(args) == 10, "please provide exactly 10 arguments to the function");
}

int main() {
    count_params(0,1,2,3,4,5,6,7,8,9);
#if SIM_ERROR
    // unblock to get an error
    count_params(1,2,3,4,5,6,7,8,9);
#endif    
    std::cout << "hello world!!!" << std::endl;
    return 0;
}