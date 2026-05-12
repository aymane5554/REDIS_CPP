#include "Server.hpp"

Val::Val()
{
    ptr = NULL;
    seconds = -1;
    type = NONE;
    prev = NULL;
    next = NULL;
}

Val::Val(const Val &obj)
{
    ptr = NULL;
    new_Val_ptr(obj);
    copy_Val_ptr(obj);
    this->seconds = obj.seconds;
    this->type = obj.type;
    prev = obj.prev;
    next = obj.next;
    key = obj.key;
}

Val &Val::operator=(const Val &obj)
{
    if (this == &obj)
        return *this;
    delete_Val_ptr();
    this->seconds = obj.seconds;
    this->type = obj.type;
    new_Val_ptr(obj);
    copy_Val_ptr(obj);
    prev = obj.prev;
    next = obj.next;
    key = obj.key;
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
    else if (type == Val::HASH)
    {
        delete static_cast<std::unordered_map<str, str> *>(ptr);
    }
    else if (type == Val::LIST)
    {
        delete static_cast<std::deque<str> *>(ptr);
    }
    ptr = NULL;
}


void Val::new_Val_ptr(const Val &obj)
{
    if (obj.type == Val::STR)
    {
        this->ptr = new str;
    }
    else if (obj.type == Val::HASH)
    {
        this->ptr = new std::unordered_map<str, str>;
    }
    else if (obj.type == Val::LIST)
    {
        this->ptr = new std::deque<str>;
    }
    else
    {
        this->ptr = NULL;
    }
}

void Val::copy_Val_ptr(const Val &obj)
{
    if (obj.ptr == NULL)
        return;
    if (obj.type == Val::STR)
    {
        *static_cast<str *>(this->ptr) = *static_cast<str *>(obj.ptr);
    }
    else if (obj.type == Val::HASH)
    {
        *static_cast<std::unordered_map<str, str> *>(this->ptr) = *static_cast<std::unordered_map<str, str> *>(obj.ptr);
    }
    else if (obj.type == Val::LIST)
        *static_cast<std::deque<str> *>(this->ptr) = *static_cast<std::deque<str> *>(obj.ptr); 
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

str Cache::Type(str &Key)
{
    std::unordered_map <str, Val>::iterator it;

    it = map.find(Key);
    if (it == map.end())
        throw ERROR("+none\r\n");
    remove(it->second);
    insert(it->second);
    if (it->second.type == Val::STR)
        return "+string\r\n";
    if (it->second.type == Val::HASH)
        return "+hash\r\n";
    return "+list\r\n";
}

long long Cache::Hset(std::vector<str> &cmd)
{
    std::unordered_map <str, Val>::iterator it;

    it = map.find(cmd[1]);
    if (it == map.end())
    {
        Val obj;
        std::unordered_map<str, str> *h;

        obj.type = Val::HASH;
        obj.ptr = new std::unordered_map<str, str>;
        h = static_cast<std::unordered_map<str, str> *>(obj.ptr);
        (*h)[cmd[2]] = cmd[3];
        it = map.insert(std::make_pair(cmd[1], obj)).first;
        it->second.key = it->first.c_str();
        insert(it->second);
        return 1;
    }
    if (it->second.type != Val::HASH)
        throw ERROR("-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
    std::unordered_map<str, str> *h = static_cast<std::unordered_map<str, str> *>(it->second.ptr);
    bool is_new_field = (h->find(cmd[2]) == h->end());
    (*h)[cmd[2]] = cmd[3];
    remove(it->second);
    insert(it->second);
    return is_new_field ? 1 : 0;
}

str Cache::Hget(std::vector<str> &cmd)
{
    std::unordered_map <str, Val>::iterator it;

    it = map.find(cmd[1]);
    if (it == map.end())
        throw ERROR("$-1\r\n");
    if (it->second.type != Val::HASH)
        throw ERROR("-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
    std::unordered_map<str, str> *h = static_cast<std::unordered_map<str, str> *>(it->second.ptr);
    std::unordered_map<str, str>::iterator field_it = h->find(cmd[2]);
    if (field_it == h->end())
        throw ERROR("$-1\r\n");
    remove(it->second);
    insert(it->second);
    return field_it->second;
}

void Cache::Hgetall(str &key, str &res_buf)
{
    std::unordered_map <str, Val>::iterator it;

    it = map.find(key);
    if (it == map.end())
        throw ERROR("*0\r\n");
    if (it->second.type != Val::HASH)
        throw ERROR("-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
    std::unordered_map<str, str> *h = static_cast<std::unordered_map<str, str> *>(it->second.ptr);
    res_buf = "*" + std::to_string(h->size() * 2) + "\r\n";
    for (std::unordered_map<str, str>::iterator field_it = h->begin(); field_it != h->end(); ++field_it)
    {
        res_buf += "$" + std::to_string(field_it->first.size()) + "\r\n";
        res_buf += field_it->first + "\r\n";
        res_buf += "$" + std::to_string(field_it->second.size()) + "\r\n";
        res_buf += field_it->second + "\r\n";
    }
    remove(it->second);
    insert(it->second);
}

long long Cache::Hdel(std::vector<str> &cmd)
{
    std::unordered_map <str, Val>::iterator it;

    it = map.find(cmd[1]);
    if (it == map.end())
        return 0;
    if (it->second.type != Val::HASH)
        throw ERROR("-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
    std::unordered_map<str, str> *h = static_cast<std::unordered_map<str, str> *>(it->second.ptr);
    std::unordered_map<str, str>::iterator field_it = h->find(cmd[2]);
    if (field_it == h->end())
    {
        remove(it->second);
        insert(it->second);
        return 0;
    }
    h->erase(field_it);
    if (h->empty())
    {
        remove(it->second);
        map.erase(it);
    }
    else
    {
        remove(it->second);
        insert(it->second);
    }
    return 1;
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
        it->second.key = it->first.c_str();
        insert(it->second);
    }
    else
    {
        if (it->second.type != Val::STR)
            throw ERROR("-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");        
        remove(it->second);
        map.erase(it);
        it = map.insert(std::make_pair(cmd[1], obj)).first;
        it->second.key = it->first.c_str();
        insert(it->second);
    }
}

str Cache::Get(str &Key)
{
    std::unordered_map <str, Val>::iterator it;

    it = map.find(Key);
    if (it == map.end())
        throw ERROR("$-1\r\n");
    if (it->second.type != Val::STR)
        throw ERROR("-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
    remove(it->second);
    insert(it->second);
    return *static_cast<str *>(it->second.ptr);
}

void Cache::Del(str &Key)
{
    std::unordered_map <str, Val>::iterator it;

    it = map.find(Key);
    if (it == map.end())
    {
        throw ERROR(":0\r\n");
    }
    remove(it->second);
    map.erase(it);
}

bool Cache::Exists(str &Key)
{
    std::unordered_map <str, Val>::iterator it;

    it = map.find(Key);
    if (it == map.end())
    {
        throw ERROR(":0\r\n");
    }
    remove(it->second);
    insert(it->second);
    return true;
}

void Cache::Expire(str &Key, long long seconds)
{
    time_t time = std::time(nullptr);
    std::unordered_map <str, Val>::iterator it;

    it = map.find(Key);
    if (it == map.end())
    {
        throw ERROR(":0\r\n");
    }
    it->second.seconds = time + seconds;
    remove(it->second);
    insert(it->second);
}

long long Cache::Ttl(str &Key)
{
    std::unordered_map <str, Val>::iterator it;

    it = map.find(Key);
    if (it == map.end())
    {
        throw ERROR(":-2\r\n");
    }
    remove(it->second);
    insert(it->second);
    return std::time(NULL) -  it->second.seconds;
}

void Cache::Flush()
{
    map.clear();
}

void Cache::Lpush(std::vector<str> &cmd)
{
    std::unordered_map <str, Val>::iterator it;
    Val obj;

    it = map.find(cmd[1]);
    if (it == map.end())
    {
        obj.type = Val::LIST;
        obj.ptr = new std::deque<str>;
        for (size_t i = 2; i < cmd.size(); i++)
            static_cast<std::deque<str> *>(obj.ptr)->push_front(cmd[i]);
        it = map.insert(std::make_pair(cmd[1], obj)).first;
        it->second.key = it->first.c_str();
        insert(it->second);
    }
    else
    {
        if (it->second.type != Val::LIST)
            throw ERROR("-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
        for (size_t i = 2; i < cmd.size(); i++)
            static_cast<std::deque<str> *>(it->second.ptr)->push_front(cmd[i]);
        remove(it->second);
        insert(it->second);
    }
}

void Cache::Rpush(std::vector<str> &cmd)
{
    std::unordered_map <str, Val>::iterator it;
    Val obj;

    it = map.find(cmd[1]);
    if (it == map.end())
    {
        obj.type = Val::LIST;
        obj.ptr = new std::deque<str>;
        for (size_t i = 2; i < cmd.size(); i++)
            static_cast<std::deque<str> *>(obj.ptr)->push_back(cmd[i]);
        it = map.insert(std::make_pair(cmd[1], obj)).first;
        it->second.key = it->first.c_str();
        insert(it->second);
    }
    else
    {
        if (it->second.type != Val::LIST)
            throw ERROR("-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
        for (size_t i = 2; i < cmd.size(); i++)
            static_cast<std::deque<str> *>(it->second.ptr)->push_back(cmd[i]);
        remove(it->second);
        insert(it->second);
    }
}

void Cache::Lpop(str &key)
{
    std::unordered_map <str, Val>::iterator it;

    it = map.find(key);
    if (it == map.end())
    {
        throw ERROR(":0\r\n");
    }
    if (it->second.type != Val::LIST)
        throw ERROR("-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
    static_cast<std::deque<str> *>(it->second.ptr)->pop_front();
    remove(it->second);
    insert(it->second);
}

void Cache::Rpop(str &key)
{
    std::unordered_map <str, Val>::iterator it;

    it = map.find(key);
    if (it == map.end())
    {
        throw ERROR(":0\r\n");
    }
    if (it->second.type != Val::LIST)
        throw ERROR("-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
    static_cast<std::deque<str> *>(it->second.ptr)->pop_back();
    remove(it->second);
    insert(it->second);
}

void Cache::Lrange(std::vector<str> &cmd, str &res_buf)
{
    std::unordered_map <str, Val>::iterator it;
    std::deque<str> *l;
    long start = 0, stop = 0;
    size_t starti = 0, stopi = 0;
    size_t lsize;

    it = map.find(cmd[1]);
    if (it == map.end())
        throw ERROR("*0\r\n");
    if (it->second.type != Val::LIST)
        throw ERROR("-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
    try
    {
        start = std::stol(cmd[2]);
        stop = std::stol(cmd[3]);
    }
    catch(const std::exception& e)
    {
        throw ERROR("-ERR Bad Request\r\n");
    }

    l = static_cast<std::deque<str> *>(it->second.ptr);
    lsize = l->size();
    starti = start;
    stopi = stop;
    if (start < 0)
        starti = lsize + start;
    if (stop < 0)
        stopi = lsize + stop;
    if ((size_t)stopi >= lsize)
        stopi = lsize - 1;
    if (stopi < starti || (size_t)starti >= lsize)
    {
        res_buf = "*0\r\n";
        return ;
    }
    remove(it->second);
    insert(it->second);
    res_buf = "*" + std::to_string((stopi - starti) + 1) + "\r\n";
    for (size_t i = starti; i <= stopi; i++)
    {
        res_buf += "$" + std::to_string((*l)[i].size()) + "\r\n";
        res_buf += (*l)[i] + "\r\n";
    }
}

Cache::Cache()
{
    front = new Val();
    back = front;
}

Cache::~Cache()
{
    delete front;
}
