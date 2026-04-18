#include "Server.hpp"

Val::Val()
{
    ptr = NULL;
    seconds = -1;
    type = NONE;
}

Val::Val(const Val &obj)
{
    new_Val_ptr(obj);
    copy_Val_ptr(obj);
    this->seconds = obj.seconds;
    this->type = obj.type;
}

Val &Val::operator=(const Val &obj)
{
    new_Val_ptr(obj);
    copy_Val_ptr(obj);
    this->seconds = obj.seconds;
    this->type = obj.type;
    return *this;
}

Val::~Val()
{
    delete_Val_ptr();
}

void Val::delete_Val_ptr()
{
    if (ptr == NULL)
        return ;
    if (type == Val::STR)
    {
        delete static_cast<str *>(ptr);
    }
    else if (type == Val::LIST)
    {
        delete static_cast<std::list<str> *>(ptr);
    }
    ptr = NULL;
}


void Val::new_Val_ptr(const Val &obj)
{
    if (obj.type == Val::STR)
    {
        this->ptr = new str;
    }
    else if (obj.type == Val::LIST)
    {
        this->ptr = new std::list<str>;
    }
}

void Val::copy_Val_ptr(const Val &obj)
{
    if (obj.type == Val::STR)
    {
        *static_cast<str *>(this->ptr) = *static_cast<str *>(obj.ptr);
    }
    else if (obj.type == Val::LIST)
        *static_cast<std::list<str> *>(this->ptr) = *static_cast<std::list<str> *>(obj.ptr); 
}

void Cache::check_expired_values()
{
    time_t ti = std::time(NULL);
    for (auto it = map.begin(); it != map.end(); )
    {
        if (it->second.seconds < ti && it->second.seconds > -1)
        {
            it = map.erase(it);
            continue;
        }
        ++it;
    }
}

void Cache::Set(std::vector<str> &cmd)
{
    Val obj;
    std::unordered_map <str, Val>::iterator it;

    obj.type = Val::STR;
    obj.ptr = new str(cmd[2]);
    it = map.find(cmd[1]);
    if (it == map.end())
    {
        it = map.insert(std::make_pair(cmd[1], obj)).first;
        recent_usage.push_back(it->first.c_str());
        it->second.recent_usage_idx = recent_usage.size() - 1;
    }
    else
    {
        recent_usage.erase(recent_usage.begin()+ it->second.recent_usage_idx);
        map.erase(it);
        it = map.insert(std::make_pair(cmd[1], obj)).first;
        recent_usage.push_back(it->first.c_str());
        it->second.recent_usage_idx = recent_usage.size() - 1;
    }
}

str Cache::Get(str Key)
{
    std::unordered_map <str, Val>::iterator it;

    it = map.find(Key);
    if (it == map.end())
        throw ERROR("$-1\r\n");
    if (it->second.type != Val::STR)
        throw ERROR("-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
    recent_usage.erase(recent_usage.begin()+ it->second.recent_usage_idx);
    recent_usage.push_back(it->first.c_str());
    it->second.recent_usage_idx = recent_usage.size() - 1;
    return *static_cast<str *>(it->second.ptr);
}

void Cache::Del(str Key)
{
    std::unordered_map <str, Val>::iterator it;

    it = map.find(Key);
    if (it == map.end())
    {
        throw ERROR(":0\r\n");
    }
    recent_usage.erase(recent_usage.begin()+ it->second.recent_usage_idx);
    map.erase(it);
}

bool Cache::Exists(str Key)
{
    std::unordered_map <str, Val>::iterator it;

    it = map.find(Key);
    if (it == map.end())
    {
        throw ERROR(":0\r\n");
    }
    recent_usage.erase(recent_usage.begin()+ it->second.recent_usage_idx);
    recent_usage.push_back(it->first.c_str());
    it->second.recent_usage_idx = recent_usage.size() - 1;
    return true;
}

void Cache::Expire(str Key, long long seconds)
{
    time_t time = std::time(nullptr);
    std::unordered_map <str, Val>::iterator it;

    it = map.find(Key);
    if (it == map.end())
    {
        throw ERROR(":0\r\n");
    }
    it->second.seconds = time + seconds;
    recent_usage.erase(recent_usage.begin()+ it->second.recent_usage_idx);
    recent_usage.push_back(it->first.c_str());
    it->second.recent_usage_idx = recent_usage.size() - 1;
}

long long Cache::Ttl(str Key)
{
    std::unordered_map <str, Val>::iterator it;

    it = map.find(Key);
    if (it == map.end())
    {
        throw ERROR(":-2\r\n");
    }
    recent_usage.erase(recent_usage.begin()+ it->second.recent_usage_idx);
    recent_usage.push_back(it->first.c_str());
    it->second.recent_usage_idx = recent_usage.size() - 1;
    return it->second.seconds;
}

void Cache::Flush()
{
    map.clear();
    recent_usage.clear();
}

Cache::Cache()
{
}

Cache::~Cache()
{
}
