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

    std::cout << "Deserialize" << std::endl;
    if (access("costum.db", F_OK))
    {
        std::cout << "costumd.db does not exist" << std::endl;
        return;
    }
    fd = open("costum.db", O_RDONLY);
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
        }
        delete[] key;
    }
    std::unordered_map <str, Val>::iterator iterator;
    read(fd, &len, 4);
    for (int i = 0; i < len; i++)
    {
        read(fd, &iterator, sizeof(std::unordered_map <str, Val>::iterator));
        // std::cout << "deserialized key" << iterator->first << std::endl;
        recent_usage.push_back(iterator);
    }
    close(fd);
    std::cout << "End of Deserialization" << std::endl;
}

void Cache::Serialize()
{
    std::cout << "Serialize" << std::endl;
    int fd = open("costum.db", O_CREAT | O_TRUNC | O_RDWR, 0777);
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
    for (auto i = map.begin(); i != map.end(); i++)
    {
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
    }
    // serialize recent usage deque
    keys_number = recent_usage.size();
    write(fd, &keys_number, 4);
    for (std::deque <std::unordered_map <str, Val>::iterator>::iterator it = recent_usage.begin(); it != recent_usage.end(); it++)
    {
        std::unordered_map <str, Val>::iterator iterator = *it;
        // std::cout << "serialized key " << iterator->first << std::endl;
        write(fd, &iterator, sizeof(std::unordered_map <str, Val>::iterator));
    }
    close(fd);
}
