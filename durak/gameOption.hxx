#ifndef DAA68818_EE93_4A58_9794_091F4C795F8E
#define DAA68818_EE93_4A58_9794_091F4C795F8E
#include "durak/card.hxx"
#include "durak/constant.hxx"
#include "durak/gameData.hxx"
#include <boost/fusion/adapted/boost_tuple.hpp>
#include <boost/fusion/adapted/std_pair.hpp>
#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include <boost/fusion/adapted/struct/define_struct.hpp>
#include <boost/fusion/algorithm/query/count.hpp>
#include <boost/fusion/functional.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/algorithm.hpp>
#include <boost/fusion/include/at.hpp>
#include <boost/fusion/include/boost_tuple.hpp>
#include <boost/fusion/include/count.hpp>
#include <boost/fusion/include/define_struct.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/fusion/sequence/intrinsic/at.hpp>
#include <boost/fusion/sequence/intrinsic_fwd.hpp>
#include <boost/optional.hpp>
#include <boost/tuple/tuple.hpp>
#include <optional>
#include <string>
#include <sys/types.h>
#include <tuple>
#include <vector>
namespace durak
{
struct GameOption
{
  u_int16_t maxCardValue = defaultMaxCardValue;
  u_int16_t typeCount = defaultTypeCount;
  u_int16_t numberOfCardsPlayerShouldHave = defaultNumberOfCardsPlayerShouldHave;
  u_int16_t roundToStart = defaultRoundToStart;
  boost::optional<Type> trump{};                                  // if set overrides the trump which is the last card in deck
  boost::optional<std::vector<Card>> customCardDeck{};            // if set ignores maxCardValue and typeCount
  boost::optional<std::vector<std::vector<Card>>> cardsInHands{}; // if set ignores maxCardValue and typeCount
};
}
BOOST_FUSION_ADAPT_STRUCT (durak::GameOption,maxCardValue,typeCount,numberOfCardsPlayerShouldHave,roundToStart,trump,customCardDeck,cardsInHands)

#endif /* DAA68818_EE93_4A58_9794_091F4C795F8E */
