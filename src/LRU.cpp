#include "Server.hpp"

void Cache::LRU()
{
    const char *s;

    std::cout << "LRU executing" << std::endl;
    if (front != NULL && front->next != NULL)
    {
        s = front->next->key;
        remove(*front->next);
        map.erase(s);
    }
}

void Cache::insert(Val &val)
{
    back->next = &val;
    val.prev = back;
    val.next = NULL;
    back = &val;
}

void Cache::remove(Val &val)
{
    if (val.next)
        val.next->prev = val.prev;
    val.prev->next = val.next;
    if (&val == back)
        back = val.prev;
}
