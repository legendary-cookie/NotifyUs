#pragma once

#include <thread>

class TCPServer {
public:
    static const int PORT = 10666;

    // starts server
    void start();

    // stops server
    void stop();

    // joining server thread - waits for server to end
    void join();

    // constructor automatically starts server thread
    TCPServer();
    ~TCPServer();

private:
    // event file descriptor - it will be used to tell server to stop
    int efd;

    // server thread
    std::thread m_thread;

    // it works in server thread
    void threadFunc();
};
