#pragma once

#include <thread>

class ConnectionHandler {
private:
    std::thread m_thread;
    int fd = -1;
    bool m_terminate = false;

    std::string readMessage();
    void sendMessage(const std::string& msg);

    void stop();

public:
    explicit ConnectionHandler(int fd);
    ~ConnectionHandler();

    void terminate();
    void threadFunc();
};
