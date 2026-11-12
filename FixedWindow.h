
#ifndef FIXED_WINDOW_H
#define FIXED_WINDOW_H

#include "RateLimiter.h"

class FixedWindow : public RateLimiter
{
private:
    int limit;
    int windowSeconds;

public:
    FixedWindow(int limit, int windowSeconds);

    bool allowRequest(const std::string& clientId) override;
};

#endif
