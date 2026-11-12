
#include "crow.h"
#include <cstdlib>
#include <string>
#include "FixedWindow.h"
#include "SlidingWindow.h"
#include "TokenBucket.h"
#include "LeakyBucket.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

int main()
{
    crow::SimpleApp app;

    FixedWindow limiter(5, 60);
    SlidingWindow slidingLimiter(5, 60);
    TokenBucket tokenLimiter(5, 1);
    LeakyBucket leakyLimiter(5, 1);
    CROW_ROUTE(app, "/")([]()
                         {

    std::ifstream file("public/index.html");

    if (!file.is_open()) {
        return crow::response(404, "index.html not found");
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    crow::response response(buffer.str());
    response.set_header("Content-Type", "text/html");

    return response; });
    CROW_ROUTE(app, "/style.css")([]()
                                  {

    std::ifstream file("public/style.css");

    if (!file.is_open()) {
        return crow::response(404, "style.css not found");
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    crow::response response(buffer.str());
    response.set_header("Content-Type", "text/css");

    return response; });
    CROW_ROUTE(app, "/script.js")([]()
                                  {

    std::ifstream file("public/script.js");

    if (!file.is_open()) {
        return crow::response(404, "script.js not found");
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    crow::response response(buffer.str());
    response.set_header("Content-Type", "application/javascript");

    return response; });

    CROW_ROUTE(app, "/count/<string>")
    ([&limiter](const std::string &clientId)
     {
        bool allowed = limiter.allowRequest(clientId);

        if (!allowed)
        {
            return crow::response(
                429,
                "Your request has been denied. Please try again later."
            );
        }

        return crow::response(
            200,
            "Request allowed! Client: " + clientId
        ); });

    CROW_ROUTE(app, "/sliding/<string>")
    ([&slidingLimiter](const std::string &clientId)
     {
        bool allowed = slidingLimiter.allowRequest(clientId);

        if (!allowed)
        {
            return crow::response(
                429,
                "Your request has been denied. Please try again later."
            );
        }

        return crow::response(
            200,
            "Sliding Window: Request allowed! Client: " + clientId
        ); });

    CROW_ROUTE(app, "/token/<string>")
    ([&tokenLimiter](const std::string &clientId)
     {
    bool allowed = tokenLimiter.allowRequest(clientId);

    if (!allowed)
    {
        return crow::response(
            429,
            "Your request has been denied. Please try again later."
        );
    }

    return crow::response(
        200,
        "Token Bucket: Request allowed! Client: " + clientId
    ); });

    CROW_ROUTE(app, "/leaky/<string>")
    ([&leakyLimiter](const std::string &clientId)
     {
    bool allowed = leakyLimiter.allowRequest(clientId);

    if (!allowed)
    {
        return crow::response(
            429,
            "Your request has been denied. Please try again later."
        );
    }

    return crow::response(
        200,
        "Leaky Bucket: Request allowed! Client: " + clientId
    ); });

    std::cout << "Starting C++ server on port 18080...\n";

    const char *portEnv = std::getenv("PORT");
    int port = portEnv ? std::stoi(portEnv) : 18080;
    app.port(port).multithreaded().run();

    return 0;
}
