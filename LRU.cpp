#include "Server.hpp"

void Cache::LRU()
{
    std::cout << "LRU recent_usage.size =" << recent_usage.size() << std::endl;
    if (recent_usage.size() > 0)
    {
        const char *s;

        s = recent_usage.front();
        map.erase(s);
        recent_usage.pop_front();
    }
    std::cout << "End of LRU" << std::endl;
}
