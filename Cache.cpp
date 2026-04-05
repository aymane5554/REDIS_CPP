#include "Cache.hpp"

void Cache::load()
{

}
void Cache::Set(std::pair<str, str> pair)
{
    (void)pair;
}

void Cache::Get(str Key)
{
    (void)Key;
}

void Cache::Del(str Key)
{
    (void)Key;
}

bool Cache::Exists(str Key)
{
    (void)Key;
    return false;
}

void Cache::Expire(str Key, long long seconds)
{
    (void)Key;
    (void)seconds;
}

long long Cache::Ttl(str Key)
{
    (void)Key;
    return 0;
}

void Cache::Flush()
{

}

Cache::Cache()
{
    
}

Cache::~Cache()
{

}
