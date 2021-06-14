#ifndef DAA68818_EE93_4A58_9794_091F4C795F8E
#define DAA68818_EE93_4A58_9794_091F4C795F8E
#include "durak/card.hxx"
#include "durak/constant.hxx"
#include <boost/optional.hpp>
#include <vector>
namespace durak
{
struct GameOption
{
  u_int16_t maxCardValue = defaultMaxCardValue;
  u_int16_t typeCount = defaultTypeCount;
  u_int16_t numberOfCardsPlayerShouldHave = defaultNumberOfCardsPlayerShouldHave;
  u_int16_t roundToStart = defaultRoundToStart;
  boost::optional<std::vector<Card>> customCardDeck{}; // if set ignores maxCardValue and typeCount
};
}
#endif /* DAA68818_EE93_4A58_9794_091F4C795F8E */
