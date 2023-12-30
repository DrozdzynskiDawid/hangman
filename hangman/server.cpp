#include "./config/config.h"
#include "Player.h"
#include "Message.h"
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <unordered_set>
#include <random>
#include <arpa/inet.h>
#include <netdb.h>
#include <error.h>
#include <signal.h>
#include <string.h>
#include <chrono>

using namespace std;

unordered_set<Player*> players;
int serverFd;
int epollFd;
string word = "";
bool gameInProgress = false;
int playersAlive = 0;
auto startTime = chrono::system_clock::now();

void setReuseAddr(int sock){
    const int one = 1;
    int res = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    if(res) error(1,errno,"setsockopt failed");
}

void prepareServerSocket() {
    addrinfo hints{};
    hints.ai_protocol = IPPROTO_TCP;
    addrinfo *resolved;
    int res = getaddrinfo(SERVER_ADDRESS, SERVER_PORT, &hints, &resolved);
    if (res) error(1,errno,"Getaddrinfo failed: %s\n", gai_strerror(res));
    if (!resolved) error(1,0,"Empty result\n");
    serverFd = socket(resolved->ai_family, resolved->ai_socktype, resolved->ai_protocol);
    setReuseAddr(serverFd);
    int resBind = bind(serverFd, resolved->ai_addr, resolved->ai_addrlen);
    if (resBind) error(1,errno,"Failed to bind:\n");
    freeaddrinfo(resolved);
    int resListen = listen(serverFd, 5);
    if (resListen) error(1,errno,"Failed to listen");
}

void writeMessageToClient(int fd, string cmd, string msg) {
    Message message;
    message.setMsg(msg);
    message.setCmd(cmd);
    string serialized = message.serialize();
    cout << "Message to client: " << fd << " - " << serialized << endl;
    int size = serialized.size();
    const char* serializedMsg = serialized.c_str();
    write(fd, serializedMsg, size);
}

void writeMessageToAll(string cmd, string msg) {
    for (Player* p: players) {
        writeMessageToClient(p->getPlayerFd(), cmd, msg);
    }
}

string getRandomWord() {
    default_random_engine gen((random_device()()));
    int fd = open(FILE_WITH_WORDS, O_RDONLY);
    if (fd == -1) {
        perror("Błąd podczas otwierania pliku ze słowami!");
    }
    // counting lines in file
    char c;
    int lineCounter = 1, n = 0;
    while ((read(fd, &c, 1)) > 0) {
        n++;
        if (c == '\n') {
            lineCounter++;
        }
    }
    if (lineCounter == 1) {
        error(1, 0, "Plik ze słowami jest pusty!");
    }
    // getting random word
    uniform_int_distribution<uint8_t> dist(1, lineCounter);
    int random = dist(gen);
    string word;
    lseek(fd,-n,SEEK_END);
    while ((read(fd, &c, 1)) > 0) {
        if (random == 1) {
            if (c == '\n') {
                break;
            }
            word += tolower(c);
        }
        if (c == '\n' && random > 1) {
            random--;
        }
    }
    close(fd);
    return word;
}

bool findLetterInWord(char letter) {
    size_t pos = word.find(letter);
    if (pos != string::npos) {
        cout << "Letter: " << letter << " found in word: " << word << endl;
        return true;
    }
    cout << "Letter: " << letter << " not found in word: " << word << endl;
    return false;
}

void closeServer() {
    close(serverFd);
    for (Player* p: players) {
        shutdown(p->getPlayerFd(), O_RDWR);
        close(p->getPlayerFd());
        delete p;
    }
    close(epollFd);
    error(0,0,"Closing server!");
    exit(1);
}

void handleCtrlC(int signum) {
    closeServer();
}

void disconnectClient(int fd) {
    cout << "Client " << fd << " disconnected!" << endl;
    for (Player* p: players) {
        if (p->getPlayerFd() == fd) {
            delete p;
        }
    }
    epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, nullptr);
    shutdown(fd, O_RDWR);
    close(fd);
}

void startGame() {
    gameInProgress = true;
    startTime = chrono::system_clock::now();
    playersAlive = players.size();
    cout << "Game started!" << endl;
    // random word
    word = getRandomWord();
    cout << "Random word from file: " << word << endl;
    int size = word.size();
    char c = '_';
    string playerWord;
    for (int i=0; i<size; i++) {
        playerWord += c;
    }
    cout<<playerWord<<endl;
    for (Player* p: players) {
        p->setLifes(PLAYER_LIFES);
        p->setPoints(0);
        p->setPlayerWord(playerWord);
        writeMessageToClient(p->getPlayerFd(), "WORD", p->getPlayerWord());
    }
    writeMessageToAll("INFO", "GAME STARTED!");
    writeMessageToAll("INFO", "Time for game is " + to_string(GAME_TIME_MINUTES) + " minutes");
}

void endGame() {
    int maxPoints = 0;
    // getting winner or winners
    for (Player* p: players) {
        if (p->getPoints() > maxPoints) {
            maxPoints = p->getPoints();
        }
    }

    string winnersList = "";
    for (Player* p: players) {
        if (p->getPoints() == maxPoints) {
            winnersList += p->getNickname() + ",";
        }
    }

    writeMessageToAll("RESULT", winnersList);
    gameInProgress = false;
}

void sendScoreboard() {
    string score = "";
    for (Player* p: players) {
        score += p->getNickname() + "\t" + to_string(p->getLifes()) + "\t" + to_string(p->getPoints()) + "\t\t";
    }
    // cout << score << endl;
    writeMessageToAll("SCOREBOARD", score);
}

void handleClient(int fd, epoll_event ee) {
    if (ee.events & EPOLLIN) {
        cout << "Message from client: " << fd << " - ";
        char buf[256] = "";

        if (read(fd,&buf,256) <= 0) {
            disconnectClient(fd);
            return;
        }

        // creating message
        Message msg;
        msg.deserialize(buf);
        cout<< msg.getCmd()<<" "<<msg.getMsg()<<endl;

        // setting nickname
        if(msg.getCmd() == "N") {
            string nick = msg.getMsg();
            if(players.size() > 0) {
                for(Player* p: players) {
                    if (p->getNickname() == nick) {
                        writeMessageToClient(fd, "INFO", "Nickname is already taken!");  
                        disconnectClient(fd);
                        return;
                    }
                }
            }
            for(Player* p: players) { 
                if (p->getPlayerFd() == fd) {
                    p->setNickname(nick);
                }
            }
            writeMessageToClient(fd, "INFO", "Nickname OK!");  
        }

        // starting game
        if(msg.getCmd() == "S") {
            if(players.size() < MINIMAL_PLAYERS_FOR_GAME) {
                string minimal = to_string(MINIMAL_PLAYERS_FOR_GAME);
                writeMessageToClient(fd, "INFO", "You need at least " + minimal + " players to start a game!");  
            }
            else if (gameInProgress) {
                writeMessageToClient(fd, "INFO", "You need to wait because another game is actually in progress!");
                auto timeLeft = (chrono::system_clock::now().time_since_epoch() - startTime.time_since_epoch());
                int seconds = GAME_TIME_MINUTES * 60 - chrono::duration_cast<chrono::seconds>(timeLeft).count();
                writeMessageToClient(fd, "INFO", "Time in seconds left to end game: " + to_string(seconds));  
            }
            else {
                startGame();
            }
        }

        // guessing letter
        if(msg.getCmd() == "L") {
            char letter = msg.getMsg()[0];
            bool letterInWord = findLetterInWord(letter);
            if(letterInWord) {
                for(Player* p: players) { 
                    if (p->getPlayerFd() == fd) {
                        string playerWord = p->getPlayerWord();
                        if (playerWord.find(letter) == string::npos) {
                            for(int i=0; i<word.size(); i++) {
                                if (word[i] == letter && playerWord[i] != letter) {
                                    playerWord[i] = letter;
                                    p->setPoints(p->getPoints() + 1);
                                }
                            }
                            p->setPlayerWord(playerWord);
                            //cout<<p->getPoints()<<endl;
                            writeMessageToClient(fd, "WORD", p->getPlayerWord());
                            if (word == p->getPlayerWord()) {
                                writeMessageToClient(fd, "INFO", "You revealed whole word! Your final score is: " + to_string(p->getPoints()));
                                writeMessageToClient(fd, "END", "player ended game");
                                endGame();
                                return;
                            }
                        }
                        else {
                            writeMessageToClient(fd, "INFO", "You've guessed correctly this letter earlier!");
                        }
                    }
                }
            }
            else {
                string letterString = string(1, letter);
                writeMessageToClient(fd, "INFO", letterString + " - letter is not in word!");
                for(Player* p: players) { 
                    if (p->getPlayerFd() == fd) {
                        p->setLifes(p->getLifes() - 1);
                        if (p->getLifes() == 0) {
                            writeMessageToClient(fd, "INFO", "No lifes remaining! Your final score is: " + to_string(p->getPoints()) + 
                            ". Word was: " + word);
                            writeMessageToClient(fd, "END", "player lost");
                            playersAlive--;
                        }
                    }
                }
            }
            sendScoreboard();
            if (playersAlive == 0) {
                endGame();
            }
        }
    }

    if (ee.events & ~EPOLLIN) {
        disconnectClient(fd);
    }

    return;
}

void serverLoop() {
    // epoll
    epollFd = epoll_create1(0);
    epoll_event ee;
    ee.events = EPOLLIN;
    ee.data.fd = serverFd;
    epoll_ctl(epollFd, EPOLL_CTL_ADD, serverFd, &ee);

    while (true) {
        if ((chrono::system_clock::now() - startTime) >= chrono::minutes{GAME_TIME_MINUTES}
                && gameInProgress) {
            writeMessageToAll("INFO", "Time has expired! Game has ended.");
            endGame();
            continue;
        }
        int number = epoll_wait(epollFd, &ee, 1, 1000);
        if (number == -1) {
            error(0,errno,"epoll_wait failed");
        }
        if (number == 0) {
            //error(0,errno,"epoll_wait timeout");
            continue;
        }
        if (ee.events & EPOLLIN) {
            if (ee.data.fd == serverFd) {
            sockaddr_in clientAddr{};
            socklen_t clientAddrSize = sizeof(clientAddr);
            int clientFd = accept(serverFd, (sockaddr*) &clientAddr, &clientAddrSize);
            error(0,0,"clientFd: %d", clientFd);
            players.insert(new Player(clientFd));
            epoll_event ee1;
            ee1.events = EPOLLIN;
            ee1.data.fd = clientFd;
            epoll_ctl(epollFd, EPOLL_CTL_ADD, clientFd, &ee1);
            }
            else {
                handleClient(ee.data.fd, ee);
            }
        }
    }
}

int main(int argc, char* argv[]) {
    // handle ctrl+c
    signal(SIGINT, handleCtrlC);

    // server socket creating
    prepareServerSocket();
    error(0,0,"serverFd: %d", serverFd);

    serverLoop();
}