#include "./config/config.h"
#include "Player.h"
#include <iostream>
#include <stdio.h>
#include <string>
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

using namespace std;

unordered_set<Player*> players;
int serverFd;
string word = "";

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
        delete p;
    }
    error(0,0,"Closing server!");
    exit(1);
}

void handleCtrlC(int signum) {
    closeServer();
}

void handleClient(int fd, epoll_event ee) {
    if (ee.events & EPOLLIN) {
        cout << "Message from client: " << ee.data.fd;
        char buf[256];
        read(fd,&buf,256);
        printf(" %s\n", buf);
        // test finding letter
        char letter = 'a';
        bool letterInWord = findLetterInWord(letter);
        if(letterInWord) write(fd, "OK", 3);
        
        write(fd, "Hello", 5);
        return;
    }
}

void serverLoop() {
    // test word
    word = getRandomWord();
    cout << "Random word from file: " << word << endl;
    // epoll
    int epollFd = epoll_create1(0);
    epoll_event ee;
    ee.events = EPOLLIN;
    ee.data.fd = serverFd;
    epoll_ctl(epollFd, EPOLL_CTL_ADD, serverFd, &ee);

    while (true) {
        int number = epoll_wait(epollFd, &ee, 1, -1);
        if (number == -1) {
            error(0,errno,"epoll_wait failed");
        }
        if (ee.events & EPOLLIN && ee.data.fd == serverFd) {
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
        // for (Player* p: players) {
        //     cout << "Player lifes: " << p->getLifes() << "Player nick" << p->getNickname() << endl;
        // }
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