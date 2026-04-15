#include "Server.hpp"

void Cache::LRU()
{
    std::cout << "LRU --size=" << map.size() << std::endl;
    if (recent_usage.size() > 0)
    {
        recent_usage.pop_front();
    }
}
