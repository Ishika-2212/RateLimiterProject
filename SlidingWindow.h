
#ifndef SLIDING_WINDOW_H
#define SLIDING_WINDOW_H

#include "RateLimiter.h"

class SlidingWindow : public RateLimiter
{
private:
    int limit;
    int windowSeconds;

public:
    SlidingWindow(int limit, int windowSeconds);

    bool allowRequest(const std::string& clientId) override;
};

#endif
