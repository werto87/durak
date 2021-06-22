#include "durak/card.hxx"
#include "durak/gameData.hxx"
#include "durak/player.hxx"
#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include <boost/fusion/adapted/struct/define_struct.hpp>
#include <boost/fusion/algorithm/query/count.hpp>
#include <boost/fusion/functional.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/algorithm.hpp>
#include <boost/fusion/include/at.hpp>
#include <boost/fusion/include/count.hpp>
#include <boost/fusion/include/define_struct.hpp>
#include <boost/fusion/sequence/intrinsic/at.hpp>
#include <boost/fusion/sequence/intrinsic_fwd.hpp>
#include <map>
#include <variant>

namespace durak
{
typedef std::vector<std::pair<PlayerRole, std::string>> PlayerRolesPlayerId;
}
BOOST_FUSION_DEFINE_STRUCT ((durak), RoundInformation, (durak::PlayerRolesPlayerId, playerRoles))
BOOST_FUSION_DEFINE_STRUCT ((durak), StartAttack, (std::vector<durak::Card>, cards))
BOOST_FUSION_DEFINE_STRUCT ((durak), AssistAttack, (durak::PlayerRole, playerRole) (std::vector<durak::Card>, cards))
BOOST_FUSION_DEFINE_STRUCT ((durak), Pass, )
BOOST_FUSION_DEFINE_STRUCT ((durak), Defend, (durak::Card, cardToBeat) (durak::Card, card))
BOOST_FUSION_DEFINE_STRUCT ((durak), DrawCardsFromTable, )

namespace durak
{
typedef std::variant<RoundInformation, StartAttack, AssistAttack, Pass, Defend, DrawCardsFromTable> HistoryEvent;

}