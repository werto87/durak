#ifndef DA13E483_1BFA_4A04_AC49_795808169C60
#define DA13E483_1BFA_4A04_AC49_795808169C60

#include "durak/card.hxx"
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

enum struct PlayerRole
{
  attack,
  defend,
  assistAttacker,
  waiting
};
}

BOOST_FUSION_ADAPT_STRUCT (durak::Card, (u_int16_t, value) (durak::Type, type))

BOOST_FUSION_DEFINE_STRUCT ((durak), PlayerData, (std::string, name) (std::vector<boost::optional<durak::Card>>, cards) (durak::PlayerRole, playerRole))

namespace durak
{
struct GameData
{
  Type trump{};
  std::vector<std::pair<Card, boost::optional<Card>>> table{};
  std::vector<PlayerData> players{};
  size_t round{};
};
}

BOOST_FUSION_ADAPT_STRUCT (durak::GameData, trump, table, players, round)

#endif /* DA13E483_1BFA_4A04_AC49_795808169C60 */
