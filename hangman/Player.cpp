#include "Player.h"
#include "./config/config.h"
#include <string>

Player::Player(int playerFd) {
    this->playerFd = playerFd;
    this->lifes = PLAYER_LIFES;
    this->points = 0;
}

void Player::setNickname(string nickname) {
    Player::nickname = nickname;
}

void Player::setPlayerFd(int playerFd) {
    Player::playerFd = playerFd;
}

void Player::setLifes(int lifes) {
    Player::lifes = lifes;
}

void Player::setPoints(int points) {
    Player::points = points;
}

void Player::setPlayerWord(string playerWord) {
    Player::playerWord = playerWord;
}

string Player::getNickname() {
    return this->nickname;
}

int Player::getPlayerFd() {
    return this->playerFd;
}

int Player::getLifes() {
    return this->lifes;
}

int Player::getPoints() {
    return this->points;
}

string Player::getPlayerWord() {
    return this->playerWord;
}
