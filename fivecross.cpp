#include "fivecross.hpp"

#include <algorithm>
#include <random>

int suit(int c) { return (0x1C & c) >> 2; }
int number(int c) { return 0x13 & c; }
bool isValid(int c) { return c != 16 && c != 13; }

bool isMovable(int card, int shared) {
  int d = card ^ shared;
  return isValid(card) && number(d) != 0 && suit(d) != 0;
}

bool isSafe(int (&hand)[4]) {
  for (int shared = 0; shared < 16; shared++) {
    bool isPossibleShared = true;
    for (auto c : hand) {
      if (c == shared) {
        isPossibleShared = false;
        break;
      }
    }
    if (isPossibleShared == false) {
      continue;
    }

    bool isSafeShared = false;
    for (auto c : hand) {
      if (isMovable(c, shared)) {
        isSafeShared = true;
        break;
      }
    }
    if (isSafeShared == true) {
      continue;
    }
    return false;
  }
  return true;
}

bool isMovableSafely(int (&cards)[4], int shared, int index) {
  std::swap(cards[index], shared);
  int result = isSafe(cards);
  std::swap(cards[index], shared);
  return result;
}

Game::Game(unsigned int seed, Player **players) {
  for (auto i : {0, 1, 2, 3}) {
    this->players[i] = players[i];
  }
  this->prng = new std::mt19937(seed);
}

Game::~Game() {
  delete (this->prng);
  for (auto p : this->players) {
    delete (p);
  }
}

Result Game::getResult() { return this->result; }

void Game::play(int turn) {
  int heap[17];
  std::iota(heap, heap + 17, 0);
  std::shuffle(heap, heap + 17, *this->prng);
  for (auto id : {0, 1, 2, 3}) {
    this->players[id]->setId(id);
    this->players[id]->setHand(
        (int[]){heap[id], heap[id + 4], heap[id + 8], heap[id + 12]});
    this->players[id]->setSeed((*this->prng)());
  }
  this->state.shared = heap[16];
  this->state.turn = turn;

  this->result.loser = -1;
  int counter;
  for (counter = 0; counter < 100; counter++) {
    for (auto p : this->players) {
      p->seeMove(this->state);
    }
    State previousState = this->state;
    this->state.turn = players[this->state.turn]->move(this->state.shared);
    if (this->state.turn == previousState.turn ||
        !isMovable(this->state.shared, previousState.shared)) {
      this->result.loser = previousState.turn;
      break;
    }
  }
  this->result.steps = counter;
}

Player::RiskTable::RiskTable() {
  this->previousId = -1;
  for (auto i : {0, 1, 2, 3}) {
    for (auto j : {0, 1, 2, 3}) {
      this->suitRisk[i][j] = 0;
      this->numberRisk[i][j] = 0;
    }
    this->notHaveInvalid[i] = false;
  }
}

void Player::RiskTable::update(State state) {
  if (state.shared == 16) {
    return;
  }
  int t = state.turn;
  int s = suit(state.shared);
  int n = number(state.shared);
  int p = this->previousId;
  if (p != -1) {
    this->suitRisk[p][s] =
        this->suitRisk[p][s] > 0 ? this->suitRisk[p][s] - 1 : 0;
    this->numberRisk[p][n] =
        this->numberRisk[p][n] > 0 ? this->numberRisk[p][n] - 1 : 0;
  }
  this->suitRisk[t][s] += 1;
  this->numberRisk[t][n] += 1;
  this->previousId = t;
  if (this->suitRisk[t][s] == 4 || this->numberRisk[t][s] == 4) {
    this->notHaveInvalid[t] = true;
  }
}

int Player::RiskTable::getRisk(State state, bool &flagIncrement) {
  int risk = 0;
  for (auto suit : {0, 1, 2, 3}) {
    if (risk < this->suitRisk[state.turn][suit]) {
      risk = this->suitRisk[state.turn][suit];
    }
  }
  for (auto number : {0, 1, 2, 3}) {
    if (risk < this->numberRisk[state.turn][number]) {
      risk = this->numberRisk[state.turn][number];
    }
  }

  flagIncrement = risk == this->suitRisk[state.turn][suit(state.shared)] ||
                  risk == this->numberRisk[state.turn][number(state.shared)];

  return risk;
}

Player::~Player() { delete (this->prng); }

PlayerTarget::PlayerTarget(int target) { this->target = target; }

void Player::setId(int id) { this->id = id; }

void Player::setHand(const int (&hand)[4]) {
  for (auto i : {0, 1, 2, 3}) {
    this->hand[i] = hand[i];
  }
}

void Player::setSeed(unsigned int seed) { this->prng = new std::mt19937(seed); }

int Player::move(int &shared) {
  int value[4][4];
  int maxValue = -1;
  for (int turn = 0; turn < 4; turn++) {
    for (int index = 0; index < 4; index++) {
      if (turn == this->id || !isMovable(this->hand[index], shared)) {
        value[turn][index] = -1;
      } else {
        value[turn][index] = this->evaluate(turn, index, shared);
      }
      maxValue = maxValue < value[turn][index] ? value[turn][index] : maxValue;
    }
  }
  if (maxValue == -1) {
    return -1;
  }

  int moves[16];
  int argMaxTurn = -1;
  int argMaxIndex = -1;
  std::iota(moves, moves + 16, 0);
  std::shuffle(moves, moves + 16, *this->prng);
  for (int i = 0; i < 16; i++) {
    int turn = moves[i] / 4;
    int index = moves[i] % 4;
    if (value[turn][index] == maxValue) {
      argMaxTurn = turn;
      argMaxIndex = index;
      break;
    }
  }
  std::swap(this->hand[argMaxIndex], shared);
  return argMaxTurn;
}

void PlayerTarget::seeMove(State state) { this->riskTable.update(state); }

void PlayerRandom::seeMove(State state) {}

void PlayerSurvive::seeMove(State state) { this->riskTable.update(state); }

int PlayerTarget::evaluate(int turn, int index, int shared) {
  int value = 0;

  value += isMovableSafely(this->hand, shared, index) ? 16 : 0;

  bool flagIncrement;
  int risk = this->riskTable.getRisk({turn, this->hand[index]}, flagIncrement);

  if (turn == this->target && flagIncrement == true) {
    value += risk * 4;
  } else if (turn != this->target && flagIncrement == false) {
    value += risk;
  }

  return value;
}

int PlayerRandom::evaluate(int turn, int index, int shared) {
  return isMovableSafely(this->hand, shared, index) ? 1 : 0;
}

int PlayerSurvive::evaluate(int turn, int index, int shared) {
  int value = 0;

  value += isMovableSafely(this->hand, shared, index) ? 16 : 0;

  bool flagIncrement;
  int risk = this->riskTable.getRisk({turn, this->hand[index]}, flagIncrement);

  if (flagIncrement == true) {
    value += risk * 4;
  }
  value += 3 - risk;

  return value;
}
