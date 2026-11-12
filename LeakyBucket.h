
#ifndef LEAKY_BUCKET_H
#define LEAKY_BUCKET_H

#include "RateLimiter.h"

class LeakyBucket : public RateLimiter
{
private:
    int capacity;
    int leakRate;

public:
    LeakyBucket(int capacity, int leakRate);

    bool allowRequest(const std::string& clientId) override;
};

#endif
