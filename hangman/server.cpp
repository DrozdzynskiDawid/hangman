#include "./config/config.h"
#include "Player.h"
#include <iostream>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unordered_set>

using namespace std;

unordered_set<Player*> players;

string getRandomWord() {
    int fd = open(FILE_WITH_WORDS, O_RDONLY);
    if (fd == -1) {
        perror("Błąd podczas otwierania pliku ze słowami!");
    }

    char c;
    int lineCounter = 1, n = 0;
    while ((read(fd, &c, 1)) > 0) {
        n++;
        if (c == '\n') {
            lineCounter++;
        }
    }

    srand((unsigned) time(NULL));
    int random = 1 + (rand() % lineCounter);

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

int main(int argc, char* argv[]) {
    // testowi gracze
    players.insert(new Player(44));
    players.insert(new Player(1000));
    for (Player* p: players) {
        if (p->getPlayerFd() == 1000) {
            p->setNickname("abc");
        }
    }
    for (Player* p: players) {
        cout << p->getLifes() << p->getNickname() <<p->getPlayerFd()<<endl;
        delete p;
    }
    // koniec testu
    string word = getRandomWord();
    cout << "Wylosowane słowo: " << word << endl;
    cout << "Życia graczy: " << PLAYER_LIFES << endl;;
}