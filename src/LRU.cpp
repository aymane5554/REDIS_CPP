#include "Server.hpp"

void Cache::LRU()
{
    const char *s;

    std::cout << "LRU executing" << std::endl;
    if (front != 0)
    {
        s = front->key;
        remove(*front);
        map.erase(s);
    }
    std::cout << "End of LRU" << std::endl;
}

void Cache::insert(Val &val)
{
    std::cout << "INSERT " << val.key << " IN LRU LIST" << std::endl;
    for (Val *iit = front->next; iit != NULL; iit = iit->next)
    {
        std::cout << iit->key << std::endl;
    }
    back->next = &val;
    val.prev = back;
    back = &val;
}

void Cache::remove(Val &val)
{
    std::cout << "REMOVE " << val.key << " FROM LRU LIST" << std::endl;
    for (Val *iit = front->next; iit != NULL; iit = iit->next)
    {
        std::cout << iit->key << std::endl;
    }
    if (val.next)
        val.next->prev = val.prev;
    val.prev->next = val.next;
    if (&val == back)
        back = val.prev;
}
