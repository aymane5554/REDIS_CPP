#include "Server.hpp"

#define MAGIC_NUMBER "\x00\x46\x54\x52\x45\x44\x49\x53"

void Cache::Deserialize()
{
    char buff[8];
    const char *buff2 = "\x00\x46\x54\x52\x45\x44\x49\x53";
    int keys_num = 0;
    int fd = 0;
    int len = 0;
    u_char is_ttl = 0;
    Val obj;
    char *key = NULL;
    char *value = NULL;

    std::cout << "Deserialize" << std::endl;
    if (access("costum.db", F_OK))
        return;
    fd = open("costum.db", O_RDONLY);
    if (fd < 0)
    {
        perror("Serialization Failed");
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
        read(fd, &len, 4);
        key = (char *)malloc((len + 1) * sizeof(char));
        read(fd, key, len);
        key[len] = '\0';
        if (obj.type == Val::STR)
        {
            read(fd, &len, 4);  
            value = (char *)malloc((len + 1) * sizeof(char));
            read(fd, value, len);
            value[len] = '\0';
            obj.ptr = new str(value);
            map.insert(std::make_pair(key, obj));
            obj.delete_Val_ptr();
            free(value);
        }
        free(key);
    }
    close(fd);
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
    close(fd);
}
