#ifndef _FIVECROSS_HPP
#define _FIVECROSS_HPP

#include <random>

struct State {
  int turn;
  int shared;
};

struct Result {
  int loser;
  int steps;
};

class Player {
public:
  virtual ~Player();
  void setId(int playerId);
  void setHand(const int (&hand)[4]);
  void setSeed(unsigned int seed);
  int move(int &shared);
  virtual void seeMove(State state) = 0;

protected:
  int hand[4];
  struct RiskTable {
    RiskTable();
    void update(State state);
    int getRisk(State state, bool &flagIncrement);
    int previousId;
    int suitRisk[4][4];
    int numberRisk[4][4];
    bool notHaveInvalid[4];
  } riskTable;

private:
  virtual int evaluate(int turn, int index, int shared) = 0;
  int id;
  std::mt19937 *prng;
};

class PlayerTarget : public Player {
public:
  PlayerTarget(int target);
  void seeMove(State state);

private:
  int target;
  int evaluate(int turn, int index, int shared);
};

class PlayerRandom : public Player {
public:
  void seeMove(State state);

private:
  int evaluate(int turn, int index, int shared);
};

class PlayerSurvive : public Player {
public:
  void seeMove(State state);

private:
  int evaluate(int turn, int index, int shared);
};

class Game {
public:
  Game(unsigned int seed, Player *players[4]);
  ~Game();
  void play(int turn);
  Result getResult();

private:
  Player *players[4];
  std::mt19937 *prng;
  State state;
  Result result;
};

#endif
