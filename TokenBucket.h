
#ifndef TOKEN_BUCKET_H
#define TOKEN_BUCKET_H

#include "RateLimiter.h"

class TokenBucket : public RateLimiter
{
private:
    int capacity;
    int refillRate;

public:
    TokenBucket(int capacity, int refillRate);

    bool allowRequest(const std::string& clientId) override;
};

#endif
