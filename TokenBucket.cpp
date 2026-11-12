
#include "TokenBucket.h"
#include <hiredis/hiredis.h>
#include <cstdlib>
#include <string>

TokenBucket::TokenBucket(int capacity, int refillRate)
    : capacity(capacity), refillRate(refillRate)
{
}

bool TokenBucket::allowRequest(const std::string& clientId)
{
    const char* redisHost = std::getenv("REDIS_HOST");
const char* redisPortEnv = std::getenv("REDIS_PORT");

if (redisHost == nullptr) {
    redisHost = "127.0.0.1";
}

int redisPort = redisPortEnv ? std::stoi(redisPortEnv) : 6379;

redisContext* redis = redisConnect(redisHost, redisPort);

    if (redis == nullptr || redis->err)
    {
        if (redis != nullptr)
            redisFree(redis);

        return false;
    }

    std::string key = "token-bucket:" + clientId;

    const char* script =
        "local capacity = tonumber(ARGV[1]); "
        "local refillRate = tonumber(ARGV[2]); "
        "local now = tonumber(ARGV[3]); "
        "local data = redis.call('HMGET', KEYS[1], 'tokens', 'lastRefill'); "
        "local tokens = tonumber(data[1]); "
        "local lastRefill = tonumber(data[2]); "
        "if tokens == nil then "
        "tokens = capacity; "
        "lastRefill = now; "
        "end; "
        "local elapsed = math.max(0, now - lastRefill); "
        "tokens = math.min(capacity, tokens + elapsed * refillRate / 1000); "
        "local allowed = 0; "
        "if tokens >= 1 then "
        "tokens = tokens - 1; "
        "allowed = 1; "
        "end; "
        "redis.call('HSET', KEYS[1], 'tokens', tokens, 'lastRefill', now); "
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
    std::string refillStr = std::to_string(refillRate);
    std::string nowStr = std::to_string(now);

    redisReply* reply = static_cast<redisReply*>(
        redisCommand(
            redis,
            "EVAL %s 1 %s %s %s %s",
            script,
            key.c_str(),
            capacityStr.c_str(),
            refillStr.c_str(),
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
