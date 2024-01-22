# WISIELEC

Opis:

Gracz łączy się do serwera i wysyła swój nick. Jeśli jest zajęty, serwer prosi o podanie innego aż do skutku.

Po ustaleniu nicku gracz przechodzi do pokoju, jeśli nie ma gry może ją rozpocząć jeśli w pokoju jest co najmniej trzech graczy, jeśli gra już się toczy widzi czas pozostały do jej zakończenia.

Gdy gra się rozpocznie, serwer losuje słowo, każdy gracz widzi tyle samo miejsc na litery (to samo słowo) oraz czas na odgadnięcie hasła. Gracze wysyłają do serwera litery, jeśli litera okaże się znajdować w haśle, gracz otrzymuje 1 punkt oraz odkrywa daną literę w haśle dla siebie. Jeśli litera jest błędna, gracz traci jedno życie. Gracze widzą liczbę żyć innych graczy podczas rozgrywki. Po utracie wszystkich żyć gracz odpada z rozgrywki. Oznacza to, że nie może wysyłać liter do serwera próbując zgadnąć hasło, nadal jednak ma szansę wygrać, jeśli inni nie zdobędą większej ilości punktów od niego. Na ekranie widzi czas pozostały do zakończenia gry oraz może śledzić ilość żyć innych.  Gra kończy się po upływie czasu lub kiedy ktoś zgadnie hasło. Na koniec wyświetla się nick zwycięzcy i liczba punktów pozostałych graczy.

Po zakończeniu gry, gracze wracają do pokoju i mogą rozpocząć kolejną rozgrywkę.

## Autorzy: Dawid Drożdżyński 151867, Wiktoria Dębowska 151874


### Przydatne linki lub komendy do setup:
Instalacja qt na open suse: https://wiki.qt.io/Install_Qt_5_on_openSUSE

Instalacja gcc, cmake, make na open suse: zypper install --type pattern devel_basis

Instalacja VSC -||-: https://owlhowto.com/how-to-install-visual-studio-code-on-opensuse/

Instalacja widgetów na qt6 -||-: zypper in qt6-widgets-devel
