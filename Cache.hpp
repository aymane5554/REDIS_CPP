#pragma once

#include <unordered_map>
#include <string>

typedef std::string str;

class Val
{
    public:
        typedef enum e_type
        {
            LIST,
            STR,
            NONE
        } t_type;
        Val::t_type type;
        long long seconds;
        void *ptr;

        void delete_Val_ptr();
        void new_Val_ptr(const Val &obj);
        void copy_Val_ptr(const Val &obj);
        Val();
        Val(const Val &obj);
        Val &operator=(const Val &obj);
        ~Val();
};

class Cache
{
    std::unordered_map <str, Val> map;
    public:
        void load(); // load from disk
        void Set(std::vector<str> &cmd); // SET key value → store it
        str Get(str Key); // GET key → retrieve it
        void Del(str Key); // DEL key → delete it
        bool Exists(str Key); // EXISTS key → does it exist?
        void Expire(str Key, long long seconds); // EXPIRE key seconds → auto-delete after N seconds
        long long Ttl(str Key); // TTL key → how many seconds left?
        void Flush(); // FLUSH → wipe everything
        void check_expired_values();
        void Serialize();
        void Deserialize();
        Cache();
        ~Cache();
};
