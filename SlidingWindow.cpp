
#include "SlidingWindow.h"
#include <hiredis/hiredis.h>
#include <string>

SlidingWindow::SlidingWindow(int limit, int windowSeconds)
    : limit(limit), windowSeconds(windowSeconds)
{
}

bool SlidingWindow::allowRequest(const std::string &clientId)
{
    redisContext *redis = redisConnect("127.0.0.1", 6379);

    if (redis == nullptr || redis->err)
    {
        if (redis != nullptr)
            redisFree(redis);

        return false;
    }

    std::string key = "sliding-window:" + clientId;
    std::string limitStr = std::to_string(limit);
    std::string windowStr = std::to_string(windowSeconds);

    const char *script =
        "local now = tonumber(ARGV[1]); "
        "local window = tonumber(ARGV[2]); "
        "local limit = tonumber(ARGV[3]); "
        "redis.call('ZREMRANGEBYSCORE', KEYS[1], '-inf', now - window); "
        "local count = redis.call('ZCARD', KEYS[1]); "
        "if count >= limit then return 0 end; "
        "redis.call('ZADD', KEYS[1], now, now .. '-' .. redis.call('INCR', KEYS[2])); "
        "redis.call('EXPIRE', KEYS[1], math.ceil(window / 1000)); "
        "redis.call('EXPIRE', KEYS[2], math.ceil(window / 1000)); "
        "return 1";

    redisReply *timeReply = static_cast<redisReply *>(
        redisCommand(redis, "TIME"));

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

    std::string timestamp = std::to_string(now);
    std::string counterKey = key + ":sequence";

    redisReply *reply = static_cast<redisReply *>(
        redisCommand(
            redis,
            "EVAL %s 2 %s %s %s %s %s",
            script,
            key.c_str(),
            counterKey.c_str(),
            timestamp.c_str(),
            std::to_string(windowSeconds * 1000).c_str(),
            limitStr.c_str()));

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
