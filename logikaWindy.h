#pragma once
#include <iostream>
#include<set>
#include<vector>
#include <chrono>
using namespace std::chrono;
using namespace std;

enum Kierunek
{
    dol, stop, gora
};

enum Stan
{
    spawn, czeka, jedzie, dojechal
};

time_point<steady_clock> czasZatrzymania;
bool odliczanieAktywne = false;
bool zjazdNaParter = false;

class Pasazer {
public:
    int pietroStart;
    int pietroKoniec;
    Kierunek kierunek;
    Stan stan;
    Pasazer(int pStart, int pKoniec) :pietroStart(pStart), pietroKoniec(pKoniec), stan(spawn) {
        kierunek = (pStart < pKoniec) ? gora : dol;
    }
};


class Winda {
public:
    int pietro;
    Kierunek kierunek;
    vector<Pasazer*> vectorPasazerow;
    int liczbaPasazerow;
    int waga;
    int pierwszaKolej;
    set<int> kolejkaGora;
    set<int, greater<int>> kolejkaDol;
    bool cele[5];


    Winda() :pierwszaKolej(0), pietro(0), kierunek(stop), liczbaPasazerow(0) {
        waga = liczbaPasazerow * 70;
        for (int i = 0; i < 5; i++) cele[i] = 0;
    }

    void aktualizacjaWagi() {
        liczbaPasazerow = vectorPasazerow.size();
        waga = liczbaPasazerow * 70;
    }

    void wezwij(Pasazer& pasazer) {

        if (pasazer.stan == spawn) {
            if (pasazer.kierunek == gora) {
                kolejkaGora.insert(pasazer.pietroStart);
                pasazer.stan = czeka;
            }
            else if (pasazer.kierunek == dol) {
                kolejkaDol.insert(pasazer.pietroStart);
                pasazer.stan = czeka;
            }
        }

    }


 
    void odbierz(Pasazer& pasazer) {
        if (vectorPasazerow.size() >= 8) return;
        if (pietro == pasazer.pietroStart && pasazer.stan == czeka && kierunek == pasazer.kierunek) {
            pasazer.stan = jedzie;  
            vectorPasazerow.push_back(&pasazer);
            if (pasazer.kierunek == gora) {
                kolejkaGora.erase(pasazer.pietroStart);
            }
            if (pasazer.kierunek == dol) {
                kolejkaDol.erase(pasazer.pietroStart);
            }
            if (pasazer.stan == jedzie)
                cele[pasazer.pietroKoniec] = true;
            aktualizacjaWagi();
            return;
        }
    }

    void odstaw() {
        for (auto it = vectorPasazerow.begin(); it != vectorPasazerow.end(); ) {
            if ((*it)->pietroKoniec == pietro && (*it)->stan == jedzie) {
                (*it)->stan = dojechal;
                it = vectorPasazerow.erase(it);
            }
            else {
                ++it;
            }
        }
        cele[pietro] = false;
        aktualizacjaWagi();
    }

    void ruchPierwszaKolej() {
        if (pierwszaKolej == pietro) return;
        pietro < pierwszaKolej ? pietro++ : pietro--;
    }

    void pierwszyRuchJeœliPotrzeba() {
        if (kierunek == stop) {
            if (!kolejkaGora.empty()) {
                pierwszaKolej = *kolejkaGora.begin();
                kierunek = gora;
                ruchPierwszaKolej();
            }
            else if (!kolejkaDol.empty()) {
                pierwszaKolej = *kolejkaDol.begin();
                kierunek = dol;
                ruchPierwszaKolej();
            }
        }
    }

    void ruch() {
        if (kierunek == gora) {
            for (int i = pietro; i < 5; i++) {
                if (cele[i] == true) {
                    pietro++;
                    return;
                }
            }
        }
        if (kierunek == dol) {
            for (int i = pietro; i >= 0; i--) {
                if (cele[i] == true) {
                    pietro--;
                    return;
                }
            }
        }
        if (kierunek == gora) {
            for (int kolejka : kolejkaGora) {
                if (kolejka > pietro) {
                    pietro++;
                    return;
                }
            }
        }
        if (kierunek == dol) {
            for (int kolejka : kolejkaDol) {
                if (kolejka < pietro) {
                    pietro--;
                    return;
                }
            }
        }
        kierunek = stop;
        if (vectorPasazerow.empty() && kolejkaGora.empty() && kolejkaDol.empty()) {
            if (!odliczanieAktywne) {
                czasZatrzymania = steady_clock::now();
                odliczanieAktywne = true;
                return;
            }

            auto teraz = steady_clock::now();
            auto uplynelo = duration_cast<seconds>(teraz - czasZatrzymania).count();
            if (uplynelo >= 5 && pietro != 0) {
                zjazdNaParter = true;
                kierunek = dol;
            }
        }
        if (!vectorPasazerow.empty() || !kolejkaGora.empty() || !kolejkaDol.empty()) {
            odliczanieAktywne = false;
            zjazdNaParter = false;
        }
        if (zjazdNaParter) {
            if (pietro > 0) {
                pietro--;
                return;
            }
            else {
                zjazdNaParter = false;
                odliczanieAktywne = false;
                kierunek = stop;
                return;
            }
        }
    }

};