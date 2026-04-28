#include "Server.hpp"
#include <cerrno>
#include <cstdlib>
#include <limits>

std::atomic<bool> sigint;

static void print_usage(const char *bin)
{
    std::cerr
        << "Usage: " << bin << " [options]\n"
        << "Options:\n"
        << "  --port <1-65535>              Listening TCP port (default: " << DEFAULT_PORT << ")\n"
        << "  --memory-mb <positive-int>     Address-space limit in MB (default: " << DEFAULT_MEMORY_LIMIT_MB << ")\n"
        << "  --ttl-interval <seconds>       TTL sweep interval (default: " << DEFAULT_TTL_SWEEP_SECONDS << ")\n"
        << "  --snapshot-interval <seconds>  Snapshot interval (default: " << DEFAULT_SNAPSHOT_SECONDS << ")\n"
        << "  --client-timeout <seconds>     Idle client timeout (default: " << DEFAULT_CLIENT_TIMEOUT_SECONDS << ")\n"
        << "  --db-file <path>               Snapshot DB file path (default: " << DEFAULT_DB_FILE << ")\n"
        << "  --wal-file <path>              WAL file path (default: " << DEFAULT_WAL_FILE << ")\n"
        << "  --help                         Show this help\n";
}

static bool parse_positive_int(const char *raw, int &out)
{
    char *end = NULL;
    errno = 0;
    long value = std::strtol(raw, &end, 10);

    if (errno != 0 || end == raw || *end != '\0')
        return false;
    if (value <= 0 || value > std::numeric_limits<int>::max())
        return false;
    out = static_cast<int>(value);
    return true;
}

static bool parse_port(const char *raw, int &out)
{
    int port;

    if (!parse_positive_int(raw, port))
        return false;
    if (port > 65535)
        return false;
    out = port;
    return true;
}

static bool parse_positive_ull(const char *raw, unsigned long long &out)
{
    char *end = NULL;
    errno = 0;
    unsigned long long value = std::strtoull(raw, &end, 10);

    if (errno != 0 || end == raw || *end != '\0')
        return false;
    if (value == 0)
        return false;
    out = value;
    return true;
}

static bool set_as_limit_bytes(rlim_t bytes)
{
    rlimit rl;

    rl.rlim_cur = bytes;
    rl.rlim_max = bytes;
    if (setrlimit(RLIMIT_AS, &rl) != 0) {
        return false;
    }
    return true;
}

int main(int argc, char **argv)
{
    RuntimeConfig config;
    unsigned long long memory_limit_mb = DEFAULT_MEMORY_LIMIT_MB;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (arg == "--help")
        {
            print_usage(argv[0]);
            return 0;
        }
        if (i + 1 >= argc)
        {
            std::cerr << "Missing value for option: " << arg << "\n";
            print_usage(argv[0]);
            return 1;
        }

        const char *raw_value = argv[++i];
        if (arg == "--port")
        {
            if (!parse_port(raw_value, config.port))
            {
                std::cerr << "Invalid --port value: " << raw_value << "\n";
                return 1;
            }
        }
        else if (arg == "--memory-mb")
        {
            if (!parse_positive_ull(raw_value, memory_limit_mb))
            {
                std::cerr << "Invalid --memory-mb value: " << raw_value << "\n";
                return 1;
            }
        }
        else if (arg == "--ttl-interval")
        {
            if (!parse_positive_int(raw_value, config.ttl_sweep_seconds))
            {
                std::cerr << "Invalid --ttl-interval value: " << raw_value << "\n";
                return 1;
            }
        }
        else if (arg == "--snapshot-interval")
        {
            if (!parse_positive_int(raw_value, config.snapshot_seconds))
            {
                std::cerr << "Invalid --snapshot-interval value: " << raw_value << "\n";
                return 1;
            }
        }
        else if (arg == "--client-timeout")
        {
            if (!parse_positive_int(raw_value, config.client_timeout_seconds))
            {
                std::cerr << "Invalid --client-timeout value: " << raw_value << "\n";
                return 1;
            }
        }
        else if (arg == "--db-file")
        {
            config.db_file = raw_value;
            if (config.db_file.empty())
            {
                std::cerr << "Invalid --db-file value\n";
                return 1;
            }
        }
        else if (arg == "--wal-file")
        {
            config.wal_file = raw_value;
            if (config.wal_file.empty())
            {
                std::cerr << "Invalid --wal-file value\n";
                return 1;
            }
        }
        else
        {
            std::cerr << "Unknown option: " << arg << "\n";
            print_usage(argv[0]);
            return 1;
        }
    }

    Server server(config);

    signal(SIGPIPE, SIG_IGN);
    if (!set_as_limit_bytes(static_cast<rlim_t>(memory_limit_mb) * 1024ULL * 1024ULL))
    {
        perror("setrlimit");
        return 1;
    }
    server.run();
    return 0;
}