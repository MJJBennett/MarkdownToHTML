#include "include/mdconverter.hpp"
#include <queue>

int main(int argc, char* argv[]) 
{
    std::deque<std::string> q;
    q.emplace_back("## Hello!");
    q.emplace_back("#It's me you are looking for :)");
    q.emplace_back("####### It's me you are looking for :)");
    q.emplace_back("# It's me you are looking for :)");

    auto res = mdc::convert_collection(q, mdc::MDHandler<std::string>());

    for (auto&& s : res) std::cout << s << std::endl;

    return 0;
}
