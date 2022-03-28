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
operator== (durak::Defend const &x, durak::Defend const &y)
{
  return x.card == y.card && x.cardToBeat == y.cardToBeat;
}
inline bool
operator!= (durak::Defend const &x, durak::Defend const &y)
{
  return !(x == y);
}

template <class T>
bool
are_equivalent (T const &l, T const &r)
{
  return l == r;
}

bool inline are_equivalent (durak::HistoryEvent const &l, durak::HistoryEvent const &r)
{
  return (l.index () == r.index ()) && std::visit ([] (auto const &l, auto const &r) { return are_equivalent (l, r); }, l, r);
}

inline bool
operator== (durak::HistoryEvent const &x, durak::HistoryEvent const &y)
{
  return are_equivalent (x, y);
}
