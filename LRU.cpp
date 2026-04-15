#include "Server.hpp"

void Cache::LRU()
{
    std::cout << "LRU --size=" << recent_usage.size() << std::endl;
    if (recent_usage.size() > 0)
    {
        recent_usage.pop_front();
    }
}
