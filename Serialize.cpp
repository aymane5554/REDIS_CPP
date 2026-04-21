#include "Server.hpp"

#define MAGIC_NUMBER "\x00\x46\x54\x52\x45\x44\x49\x53"

bool inserted(std::unordered_map<str, Val> &map, char *key, Val &obj)
{
    try
    {
        map.insert(std::make_pair(key, obj));
    }
    catch (std::exception &e)
    {
        return false;
    }
    return true;
}

bool pushed(std::deque<str> *list, char *val)
{
    try
    {
        list->push_back(val);
    }
    catch (std::exception &e)
    {
        return false;
    }
    return true;
}

void Cache::Deserialize()
{
    char buff[8];
    const char *buff2 = "\x00\x46\x54\x52\x45\x44\x49\x53";
    int keys_num = 0;
    int fd = 0;
    int len = 0;
    int klen = 0;
    u_char is_ttl = 0;
    Val obj;
    char *key = NULL;
    char *value = NULL;
    std::unordered_map<str, Val>::iterator obj_it;

    std::cout << "Deserialize" << std::endl;
    if (access(DB_FILE, F_OK))
    {
        std::cout << "costumd.db does not exist" << std::endl;
        return;
    }
    fd = open(DB_FILE, O_RDONLY);
    if (fd < 0)
    {
        perror("Serialization Failed");
        exit(1);
        return ;
    }
    read(fd, buff, 8);
    if (memcmp(buff, buff2, 8))
    {
        perror("Unvalid file format");
        return ;
    }
    read(fd, &keys_num, 4);
    for (int i = 0; i < keys_num; i++)
    {
        read(fd, &is_ttl, 1);
        if (is_ttl)
            read(fd, &obj.seconds, 8);
        read(fd, &obj.type, 1);
        read(fd, &klen, 4);
        key = new (std::nothrow) char[klen + 1];
        while (key == NULL)
        {
            LRU();
            key = new (std::nothrow) char[klen + 1];
        }
        read(fd, key, klen);
        key[klen] = '\0';
        if (obj.type == Val::STR)
        {
            read(fd, &len, 4);
            value = new (std::nothrow) char[len + 1];
            while (value == NULL)
            {
                LRU();
                value = new (std::nothrow) char[len + 1];
            }
            read(fd, value, len);
            value[len] = '\0';
            obj.ptr = new (std::nothrow) str(value);
            while (obj.ptr == NULL)
            {
                LRU();
                obj.ptr = new (std::nothrow) str(value);
            }
            while (!inserted(map, key, obj))
            {
                LRU();
            }
            obj.delete_Val_ptr();
            delete[] value;
            obj_it = map.find(key); // to have the same key char pointer in recent_usage and in the map
            recent_usage.push_back(obj_it->first.c_str());
            obj_it->second.recent_usage_idx = recent_usage.size() - 1;
        }
        else if (obj.type == Val::LIST)
        {
            char *li_ptr;
            int li_size;
            int li_val_len;
            
            obj.ptr = new (std::nothrow) std::deque<str>;
            while (obj.ptr == NULL)
            {
                LRU();
                obj.ptr = new (std::nothrow) std::deque<str>;
            }
            read(fd, &li_size, 4);
            for (int li = 0; li < li_size; li++)
            {
                read (fd, &li_val_len, 4);
                li_ptr = new (std::nothrow) char[li_val_len + 1];
                while (li_ptr == NULL)
                {
                    LRU();
                    li_ptr = new (std::nothrow) char[li_val_len + 1];
                }
                read (fd, li_ptr, li_val_len);
                li_ptr[li_val_len] = '\0';
                while (!pushed(static_cast<std::deque<str> *>(obj.ptr), li_ptr))
                {
                    LRU();
                }
                delete[] li_ptr;
            }
            while (!inserted(map, key, obj))
            {
                LRU();
            }
            obj.delete_Val_ptr();
            obj_it = map.find(key); // to have the same key char pointer in recent_usage and in the map
            recent_usage.push_back(obj_it->first.c_str());
            obj_it->second.recent_usage_idx = recent_usage.size() - 1;
        }
        delete[] key;
    }
    close(fd);
    std::cout << "End of Deserialization" << std::endl;
}

void Cache::Serialize()
{
    std::cout << "Serialize" << std::endl;
    int fd = open(DB_FILE, O_CREAT | O_TRUNC | O_RDWR, 0777);
    if (fd < 0)
    {
        perror("Serialization Failed");
        return ;
    }
    int keys_number = map.size();
    int len;
    u_char is_ttl;
    str *s_val;

    write (fd, MAGIC_NUMBER, 8);
    write (fd, &keys_number, 4);
    for (std::deque <const char *>::iterator it = recent_usage.begin(); it != recent_usage.end(); it++)
    {
        std::unordered_map <str, Val>::iterator i = map.find(*it);
        is_ttl = (i->second.seconds == -1) ? 0 : 1;
        write (fd, &is_ttl, 1);
        if (is_ttl)
            write (fd, &i->second.seconds, 8);
        write(fd, &i->second.type, 1);
        len = i->first.length();
        write (fd, &len, 4);
        write (fd, i->first.c_str(), len);
        if (i->second.type == Val::STR)
        {
            s_val = static_cast<str *>(i->second.ptr);
            len = s_val->length();
            write (fd, &len, 4);
            write (fd, s_val->c_str(), len);
        }
        else if (i->second.type == Val::LIST)
        {
            keys_number = static_cast<std::deque <str> *>(i->second.ptr)->size();
            write (fd, &keys_number, 4);
            std::deque<str>::iterator end = static_cast<std::deque <str> *>(i->second.ptr)->end();
            for (std::deque<str>::iterator li = static_cast<std::deque <str> *>(i->second.ptr)->begin(); li != end; li++)
            {
                len = li->length();
                write (fd, &len, 4);
                write (fd, li->c_str(), len);
            }
        }
    }
    close(fd);
}
