#include "hpcr.hpp"
#include <boost/asio/ip/address.hpp>

namespace fs = std::filesystem;
using boost::asio::ip::address_v4;

// when client joins, it should join a room.
// hence a session(socket, room) will be created and this client will have a session
// now, whenever it wants to start its delivery it call call start() function, where it will listen for incoming messages and push to the message Queue of the room 
// when client wants to send message it can call session's deliver() message 
// session will call deliver() to deliver the message to the room 
// room will call write() function to write any message to the client's queue 
// It will trigger the write() for each participant except the sender itself

// Map string levels ("info") to glog's numeric levels
int logLevelFromString(const std::string &level) {
  if (level == "info") return 0;     // INFO
  if (level == "warning") return 1;  // WARNING
  if (level == "error") return 2;    // ERROR
  if (level == "fatal") return 3;    // FATAL
  return 0; // Default to INFO
}

bool checkIfDirExists(const std::string& logDir) {
  if (!fs::exists(logDir)) {
    std::cerr << "FATAL: Log directory does not exist: " << logDir << "\n";
    std::exit(1);
  }
  if (!fs::is_directory(logDir)) {
    std::cerr << "FATAL: Log path is not a directory: " << logDir << "\n";
    std::exit(1);
  }
  if (access(logDir.c_str(), W_OK) != 0) {
    std::cerr << "FATAL: Log directory is not writable: " << logDir << "\n";
    std::exit(1);
  }

  return true;
}

// Initialize spdlog using our YAML config
void initLogging(const YAML::Node &config, char* argv0) {
  std::string logDir = config["logging"]["log_dir"].as<std::string>();

  if(checkIfDirExists(logDir)) {
    google::InitGoogleLogging(argv0);

    FLAGS_log_dir = logDir;
    FLAGS_minloglevel = logLevelFromString(config["logging"]["level"].as<std::string>());
  
    // Optional: also log to stderr (helpful in dev)
    FLAGS_alsologtostderr = 1;
  }
}

void accept_connection(tcp::acceptor& acceptor) {
  // asynchronously accepts connections in the background
  acceptor.async_accept([&](boost::system::error_code ec, tcp::socket socket) {
    try {
      auto remote_ep = socket.remote_endpoint();
      LOG(INFO) << "New client connected from " 
                << remote_ep.address().to_string() << ":" << remote_ep.port();

      if(!ec) {
        // implement later        
      } else {
        LOG(ERROR) << "Connection error: " << ec.message() << '\n';
      }
    } catch (std::exception& e) {
      LOG(ERROR) << "Exception: " << e.what() << '\n';
    }

    accept_connection(acceptor);
  });
}

bool checkPort(int port) {
  try {
    return (port >= 1 && port <= 65535);
  } 
  catch (const std::exception& e) {
    std::cerr << "Invalid port: " << e.what() << '\n';
    return false;
  }
}

std::string findConfigPath(int argc, char** argv) {
  // TODO(ankush): Replace manual argument parsing with a proper 
  // CLI parser library like CLI11 or Boost.Program_options.
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--config" && i + 1 < argc)
      return argv[i + 1];
  }

  // Fallbacks
  const char* envPath = std::getenv("HPCR_CONFIG");
  if (envPath)
    return envPath;
  if (fs::exists("/etc/hpcr/config.yaml"))
    return "/etc/hpcr/config.yaml";
  if (fs::exists("config.yaml"))
    return "config.yaml";

  throw std::runtime_error("No configuration file found!");
}


int main(int argc, char *argv[]) {
  std::string configPath;
  try {
    configPath = findConfigPath(argc, argv);
    LOG(INFO) << "Using configuration file: " << configPath;
  } 
  catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return 1;
  }

  // Load configuration from YAML file.
  YAML::Node config = YAML::LoadFile(configPath);
  
  // Initialize logging based on config settings.
  initLogging(config, argv[0]);

  ServerConf conf = {
    config["server"]["port"].as<int>(),
    config["server"]["host"].as<std::string>(),
    configPath,
    config["cpu"]["worker_threads"].as<uint16_t>(),
  };

  try {
    // Checks if the configured port is valid.
    if(checkPort(conf.port)) {
      boost::asio::io_context io_context;
      tcp::endpoint endpoint(tcp::v4(), conf.port);
      tcp::acceptor acceptor(io_context, 
                             tcp::endpoint(boost::asio::ip::make_address(conf.addr),
                             conf.port));

      auto bound_ep = acceptor.local_endpoint();
      LOG(INFO) << "hpcr server running at "
                << bound_ep.address().to_string()
                << ":" << bound_ep.port();


      // Creates a thread pool of worker threads to manage concurrent connections.
      // If worker thread is not configured, by default hardware_concurrency() 
      // method assigns a default value.
      std::vector<std::thread> threadPool;
      if(conf.worker_threads == 0) {
        conf.worker_threads = std::thread::hardware_concurrency();
      }

      LOG(INFO) << "Configured worker threads: " << conf.worker_threads;
      accept_connection(acceptor);

      for(int i = 0; i < conf.worker_threads; i++) {
        threadPool.emplace_back([&io_context]() {
          io_context.run();
        });
      }

      // Join threads before exit
      for(auto& thr : threadPool) {
        thr.join();
      }
    }
    else {
      std::cerr << "Error: Invalid port number '"
                << conf.port 
                << "'. Please use a number between 1 and 65535.\n";
      return 1;   // Exit with error
    }
  }
  catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
