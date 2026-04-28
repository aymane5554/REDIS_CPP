#pragma once

#include <unordered_map>
#include <deque>
#include <string>
#include <vector>

typedef std::string str;

extern const char bad_alloc_res[55];

class Val
{
    public:
        typedef enum e_type
        {
            HASH,
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
        long long Hset(std::vector<str> &cmd);
        str Hget(std::vector<str> &cmd);
        void Hgetall(str &key, str &res_buf);
        long long Hdel(std::vector<str> &cmd);
        str Get(str &Key);
        void Del(str &Key);
        bool Exists(str &Key);
        void Expire(str &Key, long long seconds);
        long long Ttl(str &Key);
        void Flush();
        str Type(str &Key);
        void check_expired_values();
        void Serialize(const str &db_file);
        void Deserialize(const str &db_file);
        void LRU();
        Cache();
        ~Cache();
};
