#ifndef HPCR_H_
#define HPCR_H_


#include "message.hpp"

#include <deque>
#include <memory>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <filesystem>
#include <thread>
#include <atomic>


#include <glog/logging.h>
#include <yaml-cpp/yaml.h>
#include <boost/asio.hpp>
#include <boost/lockfree/queue.hpp>

using boost::asio::ip::tcp;

// Taking hardcode here but later try to fetch from config file.

enum Ip { Ipv4, Ipv6 };
enum Conn { Tcp };

// stack memory 

typedef struct ServerConf {
  // Network Configurations
  int port;
  std::string addr;
  std::string confPath;

  // Threads Configurations
  uint16_t workerThreads;

} ServerConf;

class ClientSession {
private:
  tcp::socket clientSocket;
  boost::lockfree::queue<Message> clientMessageQueue;
public:  
};

class Room {
private:
  // Lockfree queue to keep all the client messages
  boost::lockfree::queue<Message> roomMessageQueue;

};


void registerClient(tcp::socket socket, int threadId);
void accept_connection(tcp::acceptor& acceptor);


#endif // HPCR_H_
