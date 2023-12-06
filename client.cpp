#include "common.h"

// class Client{
// public:
//     Client(const char* hostname, const char* port);

//     Client(const Client& other) = delete;
//     Client& operator=(const Client& other) = delete;

//     ~Client();
// public:
//     int Start() noexcept;

//     int Disconnect() noexcept;
// private:
//     /* Creates new `addrinfo` object and fills it with connection information.
//      * @return `nullptr` on error, pointer to `addrinfo` object on success.
//     */
//     addrinfo* ResolveConnectionAddress() noexcept;

//     // @return `-1` on error, valid socket on success.
//     SOCKET CreateConnectionSocket(addrinfo* conn_addr) noexcept;

//     int SendMessage(const std::string& message) noexcept;

//     int ReceiveMessage(char* writable_buff) noexcept;
//     /* Main client application loop. Handles incoming data and accepts user input.
//      * @return `-1` on error, `0` on successful exit.
//     */
//     int HandleConnection() noexcept;
    // /* Handles user input and message-sending process.
    //  * @return `-1` on error, `0` on successful exit.
    // */ 
    // int InputHandler() noexcept;

// private:
//     const std::string remote_hostname_, remote_port_;

//     SOCKET connection_socket_;

// };

#include <thread>

using namespace std::string_literals;

class Client{
public:
    Client(const char* hostname, const char* port) : remote_hostname_(hostname), remote_port_(port) {}

    Client(const Client& other) = delete;
    Client& operator=(const Client& other) = delete;

    ~Client(){
        Disconnect();
    }
public:
    int Start() noexcept{
        std::cerr << "[Info] Starting the client.\n"s;
        
        // Get server address
        addrinfo* conn_addr = ResolveConnectionAddress();
        if (!conn_addr){
            return -1;
        }

        // Create connection socket for the client
        connection_socket_ = CreateConnectionSocket(conn_addr);
        if (!IS_VALID_SOCKET(connection_socket_)){
            return -1;
        }

        std::cerr << "[Info] Connecting to remote host ("s << remote_hostname_ << ":"s << remote_port_ << ")\n"s;
        if (connect(connection_socket_, conn_addr->ai_addr, conn_addr->ai_addrlen) == -1){
            std::cerr << "[Error] Failed to connect to the remote host: connect(): "s << std::system_category().message(GET_SOCKET_ERRNO()) << std::endl;
            return -1;
        }
        std::cerr << "[Info] Successfully connected to "s << remote_hostname_ << ":"s << remote_port_ << std::endl;

        freeaddrinfo(conn_addr); // clean up ununsed resources
        
        return HandleConnection();
    }

    void Disconnect() noexcept{
        std::cerr << "[Info] Disconnecting from the remote server.\n"s;
        CLOSE_SOCKET(connection_socket_);
        std::cerr << "[Info] Disconnected."s << std::endl;
    }
private:
    /* Creates new `addrinfo` object and fills it with connection information.
     * @return `nullptr` on error, pointer to `addrinfo` object on success.
    */
    addrinfo* ResolveConnectionAddress() noexcept{
        std::cerr << "[Debug] Resolving remote server address.\n"s;
        // Create helper and connection data structures for getaddrinfo() call
        addrinfo hints, *connection_address;
        memset(&hints, 0x00, sizeof(hints));
        hints.ai_family = AF_INET; // use IPv4 for connection
        hints.ai_socktype = SOCK_STREAM; // use TCP

        if (getaddrinfo(remote_hostname_.data(), remote_port_.data(), &hints, &connection_address) == -1){
            std::cerr << "[Error] Failed to resolve remote host address: getaddrinfo(): "s << std::system_category().message(GET_SOCKET_ERRNO()) << std::endl;
            return nullptr;
        }

        return connection_address;
    }

    // @return `-1` on error, valid socket on success.
    SOCKET CreateConnectionSocket(addrinfo* conn_addr) noexcept{
        if (!conn_addr){
            std::cerr << "[Error] CreateConnectionSocket(): conn_addr is NULL."s << std::endl;
            return -1;
        }

        std::cerr << "[Debug] Creating new connection socket.\n"s;
        SOCKET new_conn_socket = socket(conn_addr->ai_family, conn_addr->ai_socktype, conn_addr->ai_protocol);
        if (!IS_VALID_SOCKET(new_conn_socket)){
            std::cerr << "[Error] Failed to create a new connection socket: socket(): "s << std::system_category().message(GET_SOCKET_ERRNO()) << std::endl;
            return -1;
        }
        return new_conn_socket;
    }

    int SendMessage(const std::string& message) noexcept{
        std::string assembled_msg(message);
        PrependMessageLength(assembled_msg);

        int total_bytes = assembled_msg.size();
        int sent_bytes = 0;
        int sent_n;
        std::cerr << "[Debug] Sending message: '"s << assembled_msg << "'\n"s;
        while (total_bytes > sent_bytes){
            sent_n = send(connection_socket_, assembled_msg.data() + sent_bytes, total_bytes - sent_bytes, 0);
            if (sent_n == -1){
                std::cerr << "[Error] Failed to send data to the remote host: send(): "s << std::system_category().message(GET_SOCKET_ERRNO()) << std::endl;
                return sent_n;
            }
            sent_bytes += sent_n;
            std::cerr << "[Debug] Sending "s << sent_n << " bytes to the remote host\n"s;
        }

 
        return sent_bytes;
    }

    int ReceiveMessage(char* writable_buff) noexcept{

        // Receive packet length (first 4 bytes)
        char msg_len[4];
        int recv_bytes = recv(connection_socket_, msg_len, sizeof(msg_len), 0);
        if (recv_bytes <= 0){ // either client disconnect or an error
            return recv_bytes;
        }
        std::cerr << "[Debug] Received "s << recv_bytes << " bytes (packet length)\n"s;
        // Check if the message conforms to the protocol
        for (const char c : std::string(msg_len)){
            if (!std::isdigit(c)){
                std::cerr << "[Error] Failed to read data from the remote host: invalid protocol format.\n"s;
                return -1;
            }
        }

        int packet_length = std::atoi(msg_len);
        recv_bytes = recv(connection_socket_, writable_buff, packet_length, 0);
        if (recv_bytes <= 0){ // Check for errors
            return recv_bytes;
        }
        std::cerr << "[Debug] Received "s << recv_bytes << " bytes (actual packet)\n"s;

        return recv_bytes;
    }
    void PrintInputPrompt() const noexcept{
        std::cin.clear();
        std::cout << " >>> "s;
        std::cout.flush();
    }

    /* Handles user input and message-sending process.
     * @return `-1` on error, `0` on successful exit.
    */ 
    int InputHandler(){
        while (true){
            char msg_buff[MAX_DATA_BUFFER_SIZE];
            PrintInputPrompt();

            std::fgets(msg_buff, MAX_DATA_BUFFER_SIZE, stdin);
            std::string message_str(msg_buff);
            message_str.pop_back(); // fgets() adds \n char to the end of the string
            if (SendMessage(message_str) == -1){
                std::exit(1);
            }
            bzero(msg_buff, MAX_DATA_BUFFER_SIZE);
        }
    }

    /* Main client application loop. Handles incoming data and accepts user input.
     * @return `-1` on error, `0` on successful exit.
    */
    int HandleConnection() noexcept{
        std::thread input_worker_thread(&Client::InputHandler, this); // Create a new thread for reading user input
        input_worker_thread.detach();
        while (true){
            char msg_buff[MAX_DATA_BUFFER_SIZE];

            int recv_bytes = ReceiveMessage(msg_buff);
            if (recv_bytes <= 0){
                if (recv_bytes == 0){
                    std::cerr << "[Info] Remote host has closed the connection."s << std::endl;
                    std::exit(1);
                }
                else{
                    std::cerr << "[Error] Failed to receive data from the remote host: recv(): "s << std::system_category().message(GET_SOCKET_ERRNO()) << std::endl;
                    std::exit(1);
                }
            }
            std::cout << msg_buff << '\n';

            PrintInputPrompt();
            bzero(msg_buff, MAX_DATA_BUFFER_SIZE); // Clear the input from any trash values
        }
    }

private:
    const std::string remote_hostname_, remote_port_;

    SOCKET connection_socket_;

};

int main(int argc, char* argv[]){
    if (argc != 3){
        std::cerr << "[Usage] client <remote_address> <remote_port>"s << std::endl;
        return 1;
    }
#ifdef _WIN32
    WSADATA d;
    if (WSAStartup(MAKEWORD(2, 2), &d)){
        std::cerr << "[Error] Failed to initialize WinSockAPI: "s << std::system_category().message(GET_SOCKET_ERRNO()) << std::endl;
        return 1;
    }
#endif

    Client client(argv[1], argv[2]);
    if (client.Start() == -1){
        return 1;
    }
}