#include <string>

enum MessageCodes{
    RegularMessage = 0,
    Disconnect,
    CreateRoom,
    JoinRoom,
    LeaveRoom,
    RenameRoom
};

int SendMessage(const std::string& message, MessageCodes command){
    unsigned char cmd_char = static_cast<unsigned char>(command);
    
    std::string cmd_msg = cmd_char + message;
}