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

bool hashed(std::unordered_map<str, str> *hash, char *field, char *value)
{
    try
    {
        (*hash)[field] = value;
    }
    catch (std::exception &e)
    {
        return false;
    }
    return true;
}

void Cache::Deserialize(const str &db_file)
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
    if (access(db_file.c_str(), F_OK))
    {
        std::cout << "costumd.db does not exist" << std::endl;
        return;
    }
    fd = open(db_file.c_str(), O_RDONLY);
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
        obj.seconds = -1;
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
            obj_it = map.find(key);
            obj_it->second.key = obj_it->first.c_str();
            insert(obj_it->second);
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
            obj_it = map.find(key);
            obj_it->second.key = obj_it->first.c_str();
            insert(obj_it->second);
        }
        else if (obj.type == Val::HASH)
        {
            int fields_size;
            int field_len;
            int value_len;
            char *field_ptr;
            char *value_ptr;

            obj.ptr = new (std::nothrow) std::unordered_map<str, str>;
            while (obj.ptr == NULL)
            {
                LRU();
                obj.ptr = new (std::nothrow) std::unordered_map<str, str>;
            }
            read(fd, &fields_size, 4);
            for (int h = 0; h < fields_size; h++)
            {
                read(fd, &field_len, 4);
                field_ptr = new (std::nothrow) char[field_len + 1];
                while (field_ptr == NULL)
                {
                    LRU();
                    field_ptr = new (std::nothrow) char[field_len + 1];
                }
                read(fd, field_ptr, field_len);
                field_ptr[field_len] = '\0';

                read(fd, &value_len, 4);
                value_ptr = new (std::nothrow) char[value_len + 1];
                while (value_ptr == NULL)
                {
                    LRU();
                    value_ptr = new (std::nothrow) char[value_len + 1];
                }
                read(fd, value_ptr, value_len);
                value_ptr[value_len] = '\0';
                while (!hashed(static_cast<std::unordered_map<str, str> *>(obj.ptr), field_ptr, value_ptr))
                {
                    LRU();
                }
                delete[] field_ptr;
                delete[] value_ptr;
            }
            while (!inserted(map, key, obj))
            {
                LRU();
            }
            obj.delete_Val_ptr();
            obj_it = map.find(key);
            obj_it->second.key = obj_it->first.c_str();
            insert(obj_it->second);
        }
        else
        {
            perror("Serialization Failed");
            exit(1);
            return ;
        }
        delete[] key;
    }
    close(fd);
    std::cout << "End of Deserialization" << std::endl;
}

void Cache::Serialize(const str &db_file)
{
    std::cout << "Serialize" << std::endl;
    int fd = open(db_file.c_str(), O_CREAT | O_TRUNC | O_RDWR, 0777);
    if (fd < 0)
    {
        perror("Serialization Failed");
        return ;
    }
    int keys_number = map.size();
    int len;
    u_char is_ttl;
    str *s_val;
    Val *val;

    write (fd, MAGIC_NUMBER, 8);
    write (fd, &keys_number, 4);
    if (!front->next)
    {
        close(fd);
        return ;
    }
    for (Val *it = front->next; it != NULL; it = it->next)
    {
        val = it;
        is_ttl = (val->seconds == -1) ? 0 : 1;
        write (fd, &is_ttl, 1);
        if (is_ttl)
            write (fd, &val->seconds, 8);
        write(fd, &val->type, 1);
        len = strlen(val->key);
        write (fd, &len, 4);
        write (fd, val->key, len);
        if (val->type == Val::STR)
        {
            s_val = static_cast<str *>(val->ptr);
            len = s_val->length();
            write (fd, &len, 4);
            write (fd, s_val->c_str(), len);
        }
        else if (val->type == Val::LIST)
        {
            keys_number = static_cast<std::deque <str> *>(val->ptr)->size();
            write (fd, &keys_number, 4);
            std::deque<str>::iterator end = static_cast<std::deque <str> *>(val->ptr)->end();
            for (std::deque<str>::iterator li = static_cast<std::deque <str> *>(val->ptr)->begin(); li != end; li++)
            {
                len = li->length();
                write (fd, &len, 4);
                write (fd, li->c_str(), len);
            }
        }
        else if (val->type == Val::HASH)
        {
            std::unordered_map<str, str> *h = static_cast<std::unordered_map<str, str> *>(val->ptr);
            keys_number = h->size();
            write(fd, &keys_number, 4);
            for (std::unordered_map<str, str>::iterator hit = h->begin(); hit != h->end(); ++hit)
            {
                len = hit->first.length();
                write(fd, &len, 4);
                write(fd, hit->first.c_str(), len);
                len = hit->second.length();
                write(fd, &len, 4);
                write(fd, hit->second.c_str(), len);
            }
        }
    }
    close(fd);
}
