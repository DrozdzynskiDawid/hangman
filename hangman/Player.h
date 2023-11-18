#ifndef PLAYER_H
#define PLAYER_H

#include "./config/config.h"
#include <string>

using namespace std;

class Player {
    public:
        Player(int playerFd);
        string nickname;
        int playerFd;
        int lifes;
        int points;
        string playerWord;

        void setNickname(string nickname);
        void setPlayerFd(int playerFd);
        void setLifes(int lifes);
        void setPoints(int points);
        void setPlayerWord(string word);

        string getNickname();
        int getPlayerFd();
        int getLifes();
        int getPoints();
        string getPlayerWord();  
};

#endif