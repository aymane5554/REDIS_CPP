#pragma once

#include <unordered_map>
#include <deque>
#include <string>

typedef std::string str;

extern char bad_alloc_res[41];

class Val
{
    public:
        typedef enum e_type
        {
            LIST,
            STR,
            NONE
        } t_type;
        long long seconds;
        void *ptr;
        Val::t_type type;
        size_t recent_usage_idx;

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
    std::deque <const char *> recent_usage;
    public:
        void Set(std::vector<str> &cmd);
        void Lpush(std::vector<str> &cmd);
        void Rpush(std::vector<str> &cmd);
        void Lpop(str &key);
        void Rpop(str &key);
        void Lrange(std::vector<str> &cmd, str &res_buf);
        str Get(str &Key);
        void Del(str &Key);
        bool Exists(str &Key);
        void Expire(str &Key, long long seconds);
        long long Ttl(str &Key);
        void Flush();
        str Type(str &Key);
        void check_expired_values();
        void Serialize();
        void Deserialize();
        void LRU();
        Cache();
        ~Cache();
};
