#ifndef HPCR_H_
#define HPCR_H_


#include "message.hpp"

#include <deque>
#include <set>
#include <memory>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <filesystem>
#include <thread>

#include <glog/logging.h>
#include <yaml-cpp/yaml.h>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

enum Ip { Ipv4, Ipv6 };
enum Conn { Tcp };

typedef struct ServerConf {
  // Network Configurations
  int port;
  std::string addr;
  std::string conf_path;

  // Threads Configurations
  uint16_t worker_threads;

} ServerConf;

class Client {
private:
    int clientSocket;

};



#endif // HPCR_H_
