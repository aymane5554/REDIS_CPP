#include "Server.hpp"

void Cache::load()
{

}

void Cache::Set(std::vector<str> &cmd)
{
    try
    {
        Val obj;
        obj.type = Val::STR;
        obj.seconds = -1;
        obj.ptr = new str;
        map.insert(std::make_pair(cmd[0], obj));
    }
    catch (std::exception &e)
    {
        throw ERROR("-ERR Value Not Set\r\n");
    }
}

str Cache::Get(str Key)
{
    if (map.find(Key) != map.end())
        throw ERROR("$-1\r\n");
    Val &val = map[Key];
    if (val.type == Val::STR)
        throw ERROR("-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
    return *static_cast<str *>(val.ptr);
}

void Cache::Del(str Key)
{
    if (map.find(Key) != map.end())
    {
        throw ERROR(":0\r\n");
    }
    map.erase(Key);
}

bool Cache::Exists(str Key)
{
    if (map.find(Key) != map.end())
    {
        throw ERROR(":0\r\n");
    }
    return true;
}

void Cache::Expire(str Key, long long seconds)
{
    time_t time = std::time(nullptr);

    if (map.find(Key) != map.end())
    {
        throw ERROR(":0\r\n");
    }
    map[Key].seconds = time + seconds;
}

long long Cache::Ttl(str Key)
{
    if (map.find(Key) != map.end())
    {
        throw ERROR(":-2\r\n");
    }
    return map[Key].seconds;
}

void Cache::Flush()
{
    map.clear();
}

Cache::Cache()
{
    
}

Cache::~Cache()
{

}
