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
        throw std::runtime_error("Value Not Set");
    }
}

str Cache::Get(str Key)
{
    if (map.find(Key) != map.end())
        throw std::runtime_error(Key + " Does Not Exist");
    Val &val = map[Key];
    if (val.type == Val::STR)
        throw std::runtime_error("Unvalid Type");
    return *static_cast<str *>(val.ptr);
}

void Cache::Del(str Key)
{
    if (map.find(Key) != map.end())
    {
        throw std::runtime_error(Key + " Does Not Exist");
    }
    map.erase(Key);
}

bool Cache::Exists(str Key)
{
    if (map.find(Key) != map.end())
    {
        return false;
    }
    return true;
}

void Cache::Expire(str Key, long long seconds)
{
    if (map.find(Key) != map.end())
    {
        throw std::runtime_error(Key + " Does Not Exist");
    }
    map[Key].seconds = seconds;
}

long long Cache::Ttl(str Key)
{
    if (map.find(Key) != map.end())
    {
        throw std::runtime_error(Key + " Does Not Exist");
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
