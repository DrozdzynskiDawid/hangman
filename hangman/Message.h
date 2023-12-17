#ifndef MESSAGE_H
#define MESSAGE_H

#include "./config/config.h"
#include <string>

using namespace std;

class Message {
    public:
        Message() {}
    private:
        string cmd;
        string msg;
    public:
        string getCmd() {
            return this->cmd;
        }

        string getMsg() {
            return this->msg;
        }

        void setCmd(string cmd) {
            if(cmd == "INFO") {
                this->cmd = "I";
            }
            if(cmd == "NICK") {
                this->cmd = "N";
            }
            if(cmd == "START") {
                this->cmd = "S";
            }
            if(cmd == "WORD") {
                this->cmd = "W";
            }
            if(cmd == "LETTER") {
                this->cmd = "L";
            }
            if(cmd == "SCOREBOARD") {
                this->cmd = "B";
            }
            if(cmd == "END") {
                this->cmd = "E";
            }
            if(cmd == "RESULT") {
                this->cmd = "R";
            }
        }

        void setMsg(string msg) {
            this->msg = msg;
        }

        string serialize() {
            string serialized = cmd + msg + MESSAGE_DELIMITER;
            return serialized;
        }

        void deserialize(string msg) {
            this->cmd = msg[0];
            msg = msg.substr(1);
            this->msg = msg;
            string del = MESSAGE_DELIMITER;
            size_t pos = this->msg.find(del);
            if (pos != string::npos) {
                this->msg.replace(pos, del.size(), "");
            }
        }  

        size_t getSize() {
            return cmd.size() + msg.size() + string(MESSAGE_DELIMITER).size();
        }
};

#endif