#ifndef DA13E483_1BFA_4A04_AC49_795808169C60
#define DA13E483_1BFA_4A04_AC49_795808169C60

#include "confu_boost/confuBoost.hxx"
#include "durak/card.hxx"
#include <boost/fusion/adapted/boost_tuple.hpp>
#include <boost/fusion/adapted/std_pair.hpp>
#include <boost/fusion/include/boost_tuple.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/optional.hpp>
#include <boost/tuple/tuple.hpp>
#include <optional>
#include <string>
#include <sys/types.h>
#include <tuple>
#include <vector>

namespace durak
{

enum struct PlayerRole
{
  attack,
  defend,
  assistAttacker,
  waiting
};
}

BOOST_FUSION_ADAPT_STRUCT (durak::Card, (u_int16_t, value) (durak::Type, type))

// BOOST_FUSION_DEFINE_STRUCT ((durak), CardData, (u_int16_t, value) (durak::Type, type))

BOOST_FUSION_DEFINE_STRUCT ((durak), PlayerData, (std::string, name) (std::vector<boost::optional<durak::Card>>, cards) (durak::PlayerRole, playerRole))

namespace durak
{
struct GameData
{
  Type trump{};
  std::vector<std::pair<Card, boost::optional<Card>>> table{};
  std::vector<PlayerData> players{};
  bool defendingPlayerWantsToDrawCardsFromTable{};
};
}

BOOST_FUSION_ADAPT_STRUCT (durak::GameData, trump, table, players, defendingPlayerWantsToDrawCardsFromTable)

BOOST_SERIALIZATION_BOILER_PLATE (durak::Type)
BOOST_SERIALIZATION_BOILER_PLATE (durak::PlayerRole)
BOOST_SERIALIZATION_BOILER_PLATE (durak::Card)
BOOST_SERIALIZATION_BOILER_PLATE (durak::PlayerData)
BOOST_SERIALIZATION_BOILER_PLATE (durak::GameData)

#endif /* DA13E483_1BFA_4A04_AC49_795808169C60 */
