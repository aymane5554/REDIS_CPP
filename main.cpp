#include "Server.hpp"

std::atomic<int> sigint = 0;

void Cache::check_expired_values()
{
    time_t ti = std::time(NULL);
    for (auto it = map.begin(); it != map.end(); )
    {
        std::cout << "TTL" << std::endl;
        if (it->second.seconds < ti && it->second.seconds > -1)
        {
            it = map.erase(it);
            continue;
        }
        ++it;
    }
}

void Server::ttl_thread()
{
    std::unique_lock<std::mutex> lock(mtx, std::defer_lock);

    sleep(60);
    while (!sigint)
    {
        lock.lock();
        cache.check_expired_values();
        lock.unlock();
        sleep(60);
    }
}

int main()
{
    Server server;

    server.run();
    return 0;
}