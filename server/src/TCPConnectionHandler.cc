#include "TCPConnectionHandler.h"
#include <cassert>
#include <errno.h>
#include <iostream>
#include <pthread.h>
#include <sys/socket.h>
#include "common.h"

ConnectionHandler::ConnectionHandler(int fd) : fd(fd) {
  assert(!this->m_thread.joinable());

  // creating thread that handles received messages
  this->m_thread = std::thread([this]() { this->threadFunc(); });
  pthread_setname_np(this->m_thread.native_handle(), "ConnectionHandler");
}

ConnectionHandler::~ConnectionHandler() { this->stop(); }

void ConnectionHandler::stop() {
  if (this->m_thread.joinable()) {
    this->m_thread.join();
  }
}

void ConnectionHandler::threadFunc() {
  auto handle = dispatcher.appendListener(1, [&](const std::string &s) {
    this->sendMessage(s);
  });
  while (!this->m_terminate) {
    //std::string msg = this->readMessage();
  }
  dispatcher.removeListener(1, handle);
}

std::string ConnectionHandler::readMessage() {
  std::string msg(
      1024,
      '\0'); // buffor with 1024 length which is filled with NULL character

  int readBytes = recv(this->fd, msg.data(), msg.size(), 0);
  if (readBytes < 1) {
    std::cout << "Error in readMessage, readBytes: " << readBytes << std::endl;
    return "";
  }

  return msg;
}

void ConnectionHandler::sendMessage(const std::string &msg) {
  int n = send(this->fd, msg.c_str(), msg.size(), 0);
  if (n != static_cast<int>(msg.size())) {
    std::cout << "Error while sending message, message size: " << msg.size()
              << " bytes sent: " << std::endl;
  }
}

void ConnectionHandler::terminate() { this->m_terminate = true; }
