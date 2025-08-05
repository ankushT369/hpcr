#include "hpcr.hpp"

namespace fs = std::filesystem;
using boost::asio::ip::address_v4;

/*
when client joins, it should join a room.
hence a session(socket, room) will be created and this client will have a session
now, whenever it wants to start its delivery it call call start() function, where it will listen for incoming messages and push to the message Queue of the room
when client wants to send message it can call session's deliver() message
session will call deliver() to deliver the message to the room
room will call write() function to write any message to the client's queue
It will trigger the write() for each participant except the sender itself
*/

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

// Initialize glog using our YAML config
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

void Room::join(ParticipantPointer participant){
        this->participants.insert(participant);
}

void Room::leave(ParticipantPointer participant){
        this->participants.erase(participant);
}

void Room::deliver(ParticipantPointer participant, Message &message){
        messageQueue.push_back(message);
        
        while (!messageQueue.empty()) {
                Message msg = messageQueue.front();
                messageQueue.pop_front(); 

                for (ParticipantPointer _participant : participants) {
                        if (participant != _participant)
                                _participant->write(msg);
                }
        }
}

void Session::async_read() {
        auto self(shared_from_this());

        boost::asio::async_read_until(clientSocket, buffer, "\n",
                [this, self](boost::system::error_code ec, std::size_t bytes_transferred) {
                        if (!ec) {
                                std::string data(boost::asio::buffers_begin(buffer.data()), 
                                                 boost::asio::buffers_begin(buffer.data()) + bytes_transferred);
                                buffer.consume(bytes_transferred);
                                LOG(INFO) << "Received: " << data << std::endl;
                                
                                Message message(data);
                                deliver(message);
                                async_read(); 
                        }
                        else {
                                room.leave(shared_from_this());
                                if (ec == boost::asio::error::eof) {
                                        LOG(INFO) << "Connection closed by peer" << '\n';
                                }
                                else {
                                        LOG(ERROR) << "Read error: " << ec.message() << '\n';
                                }
                        }
                }
        );
}


void Session::async_write(std::string messageBody, size_t messageLength){
        auto write_handler = [&](boost::system::error_code ec, std::size_t bytes_transferred) {
                if(!ec) {
                        LOG(INFO) <<"data is written to the socket: " << '\n';
                }
                else {
                        LOG(ERROR) << "write error: " << ec.message() << '\n';
                }
        };
        boost::asio::async_write(clientSocket, boost::asio::buffer(messageBody, messageLength), write_handler);
}

void Session::start(){
        room.join(shared_from_this());
        async_read();
}


Session::Session(tcp::socket s, Room& r): clientSocket(std::move(s)), room(r){};


void Session::write(Message &message){
        messageQueue.push_back(message);

        while(messageQueue.size() != 0) {
                Message message = messageQueue.front();
                messageQueue.pop_front();
                bool header_decode = message.decodeHeader();
                if(header_decode) {
                        std::string body = message.getBody(); 
                        async_write(body, message.getBodyLength());
                }
                else {
                        LOG(WARNING) << "Message length exceeds the max length" << '\n';
                }
        }
}


void Session::deliver(Message& incomingMessage){
        room.deliver(shared_from_this(), incomingMessage);
}


void accept_connection(tcp::acceptor &acceptor, Room &room) {
        // asynchronously accepts connections in the background
        acceptor.async_accept([&](boost::system::error_code ec, tcp::socket socket) {
                LOG(INFO) << "Client accepted " << '\n';
                if(!ec) {
                        std::shared_ptr<Session> session = std::make_shared<Session>(std::move(socket), room);
                        session->start();
                }
                accept_connection(acceptor, room);
        });
}


bool check_port(int port) {
        try {
                return (port >= 1 && port <= 65535);
        } 
        catch (const std::exception& e) {
                std::cerr << "Invalid port: " << e.what() << '\n';
                return false;
        }
}

std::string findConfigPath(int argc, char** argv) {
        for (int i = 1; i < argc; ++i) {
                std::string arg = argv[i];
                if (arg == "--config" && i + 1 < argc) {
                        return argv[i + 1];
                }
        }

        // Fallbacks
        const char* envPath = std::getenv("HPCR_CONFIG");
        if (envPath) return envPath;
        if (fs::exists("/etc/hpcr/config.yaml")) return "/etc/hpcr/config.yaml";
        if (fs::exists("config.yaml")) return "config.yaml";

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

        // init resources and load configurations
        YAML::Node config = YAML::LoadFile(configPath);
        initLogging(config, argv[0]);

        ServerConf conf = {
                config["server"]["port"].as<int>(),
                config["server"]["host"].as<std::string>(),
                configPath,
        };

        try {
                if(check_port(conf.port)) {
                        LOG(INFO) << "hpcr server running at "
                                << conf.addr << ":" 
                                << conf.port;

                        Room room;
                        boost::asio::io_context io_context;
                        tcp::endpoint endpoint(tcp::v4(), conf.port);
                        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), conf.port));

                        accept_connection(acceptor, room);

                        io_context.run();
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
