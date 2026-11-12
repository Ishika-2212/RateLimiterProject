
#include "LeakyBucket.h"
#include <hiredis/hiredis.h>
#include <string>

LeakyBucket::LeakyBucket(int capacity, int leakRate)
    : capacity(capacity), leakRate(leakRate)
{
}

bool LeakyBucket::allowRequest(const std::string& clientId)
{
    redisContext* redis = redisConnect("127.0.0.1", 6379);

    if (redis == nullptr || redis->err)
    {
        if (redis != nullptr)
            redisFree(redis);

        return false;
    }

    std::string key = "leaky-bucket:" + clientId;

    const char* script =
        "local capacity = tonumber(ARGV[1]); "
        "local leakRate = tonumber(ARGV[2]); "
        "local now = tonumber(ARGV[3]); "
        "local data = redis.call('HMGET', KEYS[1], 'water', 'lastLeak'); "
        "local water = tonumber(data[1]) or 0; "
        "local lastLeak = tonumber(data[2]) or now; "
        "local elapsed = math.max(0, now - lastLeak); "
        "water = math.max(0, water - elapsed * leakRate / 1000); "
        "local allowed = 0; "
        "if water + 1 <= capacity then "
        "water = water + 1; "
        "allowed = 1; "
        "end; "
        "redis.call('HSET', KEYS[1], 'water', water, 'lastLeak', now); "
        "redis.call('EXPIRE', KEYS[1], 86400); "
        "return allowed";

    redisReply* timeReply = static_cast<redisReply*>(
        redisCommand(redis, "TIME")
    );

    if (timeReply == nullptr || timeReply->type != REDIS_REPLY_ARRAY)
    {
        if (timeReply != nullptr)
            freeReplyObject(timeReply);

        redisFree(redis);
        return false;
    }

    long long seconds = std::stoll(timeReply->element[0]->str);
    long long microseconds = std::stoll(timeReply->element[1]->str);
    long long now = seconds * 1000 + microseconds / 1000;

    freeReplyObject(timeReply);

    std::string capacityStr = std::to_string(capacity);
    std::string leakStr = std::to_string(leakRate);
    std::string nowStr = std::to_string(now);

    redisReply* reply = static_cast<redisReply*>(
        redisCommand(
            redis,
            "EVAL %s 1 %s %s %s %s",
            script,
            key.c_str(),
            capacityStr.c_str(),
            leakStr.c_str(),
            nowStr.c_str()
        )
    );

    if (reply == nullptr)
    {
        redisFree(redis);
        return false;
    }

    bool allowed = reply->type == REDIS_REPLY_INTEGER &&
                   reply->integer == 1;

    freeReplyObject(reply);
    redisFree(redis);

    return allowed;
}
