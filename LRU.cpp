#include "Server.hpp"

void Cache::LRU()
{
    std::cout << "LRU recent_usage.size =" << recent_usage.size() << std::endl;
    if (recent_usage.size() > 0)
    {
        std::unordered_map <str, Val>::iterator it;

        it = recent_usage.front();
        std::cout << "first line" << std::endl;
        map.erase(it);
        std::cout << "second line" << std::endl;
        recent_usage.pop_front();
        std::cout << "third line" << std::endl;
    }
    std::cout << "End of LRU" << std::endl;
}
