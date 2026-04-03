#pragma once

#include <unordered_map>
#include <string>

typedef std::string str;

class Cache
{
    std::unordered_map <str, str> map;
    public:
        void load(); // SET key value → store it
        void Set(std::pair<str, str> pair); // SET key value → store it
        void Get(str Key); // GET key → retrieve it
        void Del(str Key); // DEL key → delete it
        bool Exists(str Key); // EXISTS key → does it exist?
        void Expire(str Key, long long seconds); // EXPIRE key seconds → auto-delete after N seconds
        long long Ttl(str Key); // TTL key → how many seconds left?
        void Flush(); // FLUSH → wipe everything
        Cache();
        ~Cache();
};



