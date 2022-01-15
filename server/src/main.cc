#include <csignal>
#include <cstdio>
#include <cstring>
#include <eventpp/eventdispatcher.h>
#include <iostream>
#include <netinet/in.h>
#include <spdlog/spdlog.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <regex>
#include <map>
#include <httpxx/Request.hpp>

#define PORT 8000

eventpp::EventDispatcher<int, void(const std::string &)> dispatcher;

void wait(int time) {
  std::this_thread::sleep_for(std::chrono::milliseconds(time));
}

void signalHandler(int signum) {
  // cleanup and close up stuff here
  // terminate program
  dispatcher.dispatch(666, "");
  exit(signum);
}

void setupHTTPSocket(int *sockfd, socklen_t *socklen, bool *err) {
  *sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (*sockfd < 0) {
    spdlog::critical("HTTP: socket error");
    *err = true;
    return;
  }
  int yes = 1;
  if (setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
    spdlog::critical("HTTP: setsockopt error");
    *err = true;
    return;
  }
  struct sockaddr_in lst_addr;
  lst_addr.sin_family = AF_INET;
  lst_addr.sin_port = htons(PORT);
  lst_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  *socklen = sizeof(struct sockaddr_in);
  int ret = bind(*sockfd, (struct sockaddr *)&lst_addr, *socklen);
  if (ret < 0) {
    spdlog::critical("HTTP: bind error");
    *err = true;
    return;
  }
  if (listen(*sockfd, 5) < 0) {
    spdlog::critical("HTTP: listen error");
    *err = true;
    return;
  }
}


int main() {
  spdlog::info("Starting notify daemon");
  signal(SIGINT, signalHandler);
  
  if (getenv("DEBUG")) {
  	spdlog::set_level(spdlog::level::debug);
	dispatcher.appendListener(1, [](const std::string &s) { 
			spdlog::debug("RecvEvent with message: {}", s);
	});
  }
  
  std::thread httpworker([]() {
    volatile bool shouldStop = false;
    int sockfd;
    socklen_t socklen;
    bool err;

    dispatcher.appendListener(
        666, [&shouldStop](std::string) { shouldStop = true; });

    setupHTTPSocket(&sockfd, &socklen, &err);
    if (err) {
      spdlog::critical("HTTP Socket creation failed");
      shouldStop = true;
    } else {
      spdlog::info("Started HTTP worker");
    }
    while (!shouldStop) {
      struct sockaddr_in cli_addr;
      int newfd = accept(sockfd, (struct sockaddr *)&cli_addr, &socklen);
      if (newfd < 0) {
        spdlog::error("error accepting connection");
        continue;
      }
      spdlog::debug("---");
      spdlog::debug("handling new connection");
      char buff[1024] = {0};
      int ret = recv(newfd, buff, 1023, 0);
      if (ret > 0) {
	http::Request request;
	request.feed(buff, sizeof(buff));
	spdlog::debug("User-Agent: {}", request.header("User-Agent"));
	spdlog::debug("Path: {}", request.url());
	if (request.url() != "/") {
		dispatcher.dispatch(1, request.url().substr(1));
	}
      }
      std::string rsp = "200 OK";
      memset(buff, 0x00, 1024);
      sprintf(buff, "%s\r\n%s%d\r\n%s\r\n\r\n%s%", "HTTP/1.1 200 OK",
              "Content-Length: ", rsp.length(),
              "Content-Type:text/html:charset=UTF-8", rsp.c_str());
      send(newfd, buff, strlen(buff), 0);
      spdlog::debug("wrote response");
      close(newfd);
    }
    if (sockfd >= 0) {
      close(sockfd);
    }
    spdlog::info("Stopped HTTP worker");
  });

  httpworker.join();
  return 0;
}
