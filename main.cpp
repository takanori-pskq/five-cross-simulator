#include "fivecross.hpp"

#include <iostream>

int main() {
  int N = 100000;

  int numLoses[4] = {0, 0, 0, 0};
  int numDraws = 0;
  int sumSteps = 0;

  std::random_device seed;

  for (int i = 0; i < N; i++) {
    Game g(seed(), (Player *[]){new PlayerSurvive, new PlayerRandom,
                                new PlayerTarget(3), new PlayerSurvive});
    g.play(i % 4);

    auto result = g.getResult();
    result.loser == -1 ? numDraws++ : numLoses[result.loser]++;
    sumSteps += result.steps;
  }

  std::cout << "Lose Rates: " << std::endl;
  std::cout << "\tPlayer 0:\t" << (float)numLoses[0] / N << std::endl;
  std::cout << "\tPlayer 1:\t" << (float)numLoses[1] / N << std::endl;
  std::cout << "\tPlayer 2:\t" << (float)numLoses[2] / N << std::endl;
  std::cout << "\tPlayer 3:\t" << (float)numLoses[3] / N << std::endl;
  std::cout << "\tDraw:\t\t" << (float)numDraws / N << std::endl;
  std::cout << "Average Steps:\t\t" << (float)sumSteps / N << std::endl;

  return 0;
}
