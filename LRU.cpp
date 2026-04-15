#include "Server.hpp"

void Cache::LRU()
{
    if (recent_usage.size() > 0)
    {
        recent_usage.pop_front();
    }
}
