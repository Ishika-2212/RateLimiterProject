
#include "FixedWindow.h"
#include <hiredis/hiredis.h>
#include <iostream>
#include <string>

FixedWindow::FixedWindow(int limit, int windowSeconds)
    : limit(limit), windowSeconds(windowSeconds)
{
}

bool FixedWindow::allowRequest(const std::string& clientId)
{
    redisContext* redis = redisConnect("127.0.0.1", 6379);

    if (redis == nullptr || redis->err)
{
    std::cerr << "Redis connection failed: ";

    if (redis != nullptr)
    {
        std::cerr << redis->errstr << "\n";
        redisFree(redis);
    }
    else
    {
        std::cerr << "Could not allocate Redis connection\n";
    }

    return false;
}

    std::string key = "fixed-window:" + clientId;
    std::string expiry = std::to_string(windowSeconds);

    const char* script =
        "local count = redis.call('INCR', KEYS[1]); "
        "if count == 1 then "
        "redis.call('EXPIRE', KEYS[1], ARGV[1]); "
        "end; "
        "return count";

    redisReply* reply = static_cast<redisReply*>(
        redisCommand(
            redis,
            "EVAL %s 1 %s %s",
            script,
            key.c_str(),
            expiry.c_str()
        )
    );

    if (reply == nullptr)
    {
        std::cerr << "Redis command failed\n";
        redisFree(redis);
        return false;
    }

    if (reply->type != REDIS_REPLY_INTEGER)
    {
        std::cerr << "Unexpected Redis reply: "
                  << (reply->str ? reply->str : "no message") << "\n";

        freeReplyObject(reply);
        redisFree(redis);
        return false;
    }

    long long count = reply->integer;

    std::cout << "Client: " << clientId
              << " | Request count: " << count << "\n";

    freeReplyObject(reply);
    redisFree(redis);

    return count <= limit;
}
