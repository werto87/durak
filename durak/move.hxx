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
#include <cstdlib>
#include <iostream>
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
inline bool
operator== (durak::RoundInformation const &x, durak::RoundInformation const &y)
{
  return x.playerRoles == y.playerRoles;
}
inline bool
operator!= (durak::RoundInformation const &x, durak::RoundInformation const &y)
{
  return !(x == y);
}

inline bool
operator== (durak::StartAttack const &x, durak::StartAttack const &y)
{
  return x.cards == y.cards;
}
inline bool
operator!= (durak::StartAttack const &x, durak::StartAttack const &y)
{
  return !(x == y);
}
inline bool
operator== (durak::AssistAttack const &x, durak::AssistAttack const &y)
{
  return x.playerRole == y.playerRole && x.cards == y.cards;
}

inline bool
operator!= (durak::AssistAttack const &x, durak::AssistAttack const &y)
{
  return !(x == y);
}
inline bool
operator== (durak::Pass const &, durak::Pass const &)
{
  return true;
}
inline bool
operator!= (durak::Pass const &x, durak::Pass const &y)
{
  return !(x == y);
}
inline bool
operator== (durak::DrawCardsFromTable const &, durak::DrawCardsFromTable const &)
{
  return true;
}
inline bool
operator!= (durak::DrawCardsFromTable const &x, durak::DrawCardsFromTable const &y)
{
  return !(x == y);
}

inline bool
operator== (durak::HistoryEvent const &x, durak::HistoryEvent const &y)
{
  using namespace durak;
  if (x.index () != y.index ())
    {
      return false;
    }
  else
    {
      if (std::holds_alternative<RoundInformation> (x))
        {
          return std::get<RoundInformation> (x) == std::get<RoundInformation> (y);
        }
      else if (std::holds_alternative<StartAttack> (x))
        {
          return std::get<StartAttack> (x) == std::get<StartAttack> (y);
        }
      else if (std::holds_alternative<AssistAttack> (x))
        {
          return std::get<AssistAttack> (x) == std::get<AssistAttack> (y);
        }
      else if (std::holds_alternative<Pass> (x))
        {
          return std::get<Pass> (x) == std::get<Pass> (y);
        }
      else if (std::holds_alternative<DrawCardsFromTable> (x))
        {
          return std::get<DrawCardsFromTable> (x) == std::get<DrawCardsFromTable> (y);
        }
      else
        {
          std::cout << "not allowed durak::HistoryEvent" << std::endl;
          abort ();
          return false;
        }
    }
}
