#include "include/mdconverter.hpp"
#include <queue>

int main(int argc, char* argv[]) 
{
    std::deque<std::string> q;
    q.emplace_back("Hello!");
    q.emplace_back("It's me you are looking for :)");

    mdc::convert_collection(q);

    return 0;
}
