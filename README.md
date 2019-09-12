# space-invaders
Multithread Linux Terminal Space Invaders game made with C++.
The code triggers 3 threads: one for controlling the invaders movements, one to read input command from players (movement and shooting), and the last thread draws the screen at each 50ms.
The result of each session is saved on the file logger.txt

Install library dependencies:
```
sudo apt-get install libboost-dev
```

Compile:
```
g++ main.cpp -o atr -std=c++11 -pthread
```

Run:
```
./atr
```
