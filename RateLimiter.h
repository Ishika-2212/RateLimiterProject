
#ifndef RATE_LIMITER_H
#define RATE_LIMITER_H

#include <string>

class RateLimiter
{
public:
    virtual bool allowRequest(const std::string& clientId) = 0;
    virtual ~RateLimiter() = default;
};

#endif
