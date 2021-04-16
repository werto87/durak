#ifndef A85D1720_A39F_43B7_B56C_7843E3A02A0D
#define A85D1720_A39F_43B7_B56C_7843E3A02A0D
#include "src/card.hxx"
#include <algorithm>
#include <cstddef>
#include <sys/types.h>
#include <vector>
struct Player
{
public:
  void putCards (std::vector<Card> const &cardsToPut, std::vector<std::pair<Card, std::optional<Card> > > &target);

  bool dropCard (Card const &card);

  void takeCards (std::vector<Card> &&cardsToTake);

  std::vector<Card> const &getCards () const;

  std::vector<Card> cardsForIndex (std::vector<size_t> const &cardIndex) const;

private:
  std::vector<Card> cards{};
};

#endif /* A85D1720_A39F_43B7_B56C_7843E3A02A0D */
