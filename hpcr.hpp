#ifndef HPCR_H_
#define HPCR_H_


#include "message.hpp"
#include "thread-cache.hpp"

#include <deque>
#include <memory>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <filesystem>
#include <thread>
#include <atomic>
#include <array>


#include <glog/logging.h>
#include <yaml-cpp/yaml.h>
#include <boost/asio.hpp>
#include <boost/lockfree/queue.hpp>

using boost::asio::ip::tcp;

// Taking hardcode here but later try to fetch from config file.

enum Ip { Ipv4, Ipv6 };
enum Conn { Tcp };

// stack memory 
extern std::array<int, 1000000> clientSocketStack;

typedef struct ServerConf {
  // Network Configurations
  int port;
  std::string addr;
  std::string confPath;

  // Threads Configurations
  uint16_t workerThreads;

} ServerConf;

class ClientSession : public std::enable_shared_from_this<ClientSession> {
private:
  tcp::socket clientSocket;
  boost::asio::streambuf buffer;
  boost::lockfree::queue<Message> clientMessageQueue {1024};
public:
  explicit ClientSession(tcp::socket socket)
    : clientSocket(std::move(socket)) {}

  void readClient();
};

class Room {
private:
  // Lockfree queue to keep all the client messages
  boost::lockfree::queue<Message> roomMessageQueue;

};


void accept_connection(tcp::acceptor& acceptor, const std::vector<std::unique_ptr<boost::asio::io_context>>& ioContexts, CachePool<std::shared_ptr<ClientSession>>& pool);

#endif // HPCR_H_
