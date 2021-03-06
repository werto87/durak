#ifndef CB3E895C_E721_4274_A037_FDB32C4DE314
#define CB3E895C_E721_4274_A037_FDB32C4DE314
#include "durak/game.hxx"
#include <confu_json/confu_json.hxx>
#include <fmt/format.h>
#include <iostream>
#include <magic_enum.hpp>
#include <pipes/pipes.hpp>
#include <range/v3/all.hpp>
#include <sstream>
#include <vector>

// helper type for the visitor #4
template <class... Ts> struct overloaded : Ts...
{
  using Ts::operator()...;
};
// explicit deduction guide (not needed as of C++20)
template <class... Ts> overloaded (Ts...) -> overloaded<Ts...>;

namespace durak
{

struct Card;
struct Player;
class Game;

inline std::ostream &operator<< (std::ostream &os, Card const &card);

inline std::ostream &operator<< (std::ostream &os, Player const &player);

inline std::string cardsSortedByValueWithIndex (std::vector<Card> cards);

inline std::string attackingPlayerWithNameAndCardIndexValueAndType (Game const &game);

inline std::string defendingPlayerWithNameAndCardIndexValueAndType (Game const &game);

inline std::string assistingPlayerWithNameAndCardIndexValueAndType (Game const &game);

inline std::string tableAsString (Game const &game);

inline std::ostream &
operator<< (std::ostream &os, Card const &card)
{
  os << fmt::format ("Card: {{{0} , {1}}}", card.value, magic_enum::enum_name (card.type));
  return os;
}

inline std::ostream &
operator<< (std::ostream &os, Player const &player)
{
  os << "Player Cards sorted by Value" << std::endl;
  auto cards = player.getCards ();
  ranges::sort (cards);
  for (auto const &card : cards)
    os << card << std::endl;
  return os;
}

inline std::string
cardsSortedByValueWithIndex (std::vector<Card> cards)
{
  ranges::sort (cards);
  auto cardsMessage = std::stringstream{};
  cardsMessage << "Cards sorted by Value with Index, Value and Type " << std::endl;
  for (size_t i = 0; auto const &card : cards)
    {
      i++;
      cardsMessage << "index: " << i << " " << card << std::endl;
    }

  return cardsMessage.str ();
}

inline std::string
attackingPlayerWithNameAndCardIndexValueAndType (Game const &game)
{
  auto result = std::stringstream{};
  if (game.getAttackingPlayer ())
    {
      result << "Attacking Player: " << game.getAttackingPlayer ().value ().id << std::endl << "trump: " << magic_enum::enum_name (game.getTrump ()) << std::endl << cardsSortedByValueWithIndex (game.getAttackingPlayer ().value ().getCards ());
    }
  else
    {
      result << "No Attacking Player";
    }
  return result.str ();
}

inline std::string
defendingPlayerWithNameAndCardIndexValueAndType (Game const &game)
{
  auto result = std::stringstream{};
  if (game.getDefendingPlayer ())
    {
      result << "Defending Player: " << game.getDefendingPlayer ().value ().id << std::endl << "trump: " << magic_enum::enum_name (game.getTrump ()) << std::endl << cardsSortedByValueWithIndex (game.getDefendingPlayer ().value ().getCards ());
    }
  else
    {
      result << "No Defending Player";
    }

  return result.str ();
}

inline std::string
assistingPlayerWithNameAndCardIndexValueAndType (Game const &game)
{
  auto result = std::stringstream{};
  if (game.getAssistingPlayer ())
    {
      result << "Assisting Player: " << game.getAssistingPlayer ().value ().id << std::endl << "trump: " << magic_enum::enum_name (game.getTrump ()) << std::endl << cardsSortedByValueWithIndex (game.getAssistingPlayer ().value ().getCards ());
    }
  else
    {
      result << "No Assisting Player";
    }

  return result.str ();
}

inline std::ostream &
operator<< (std::ostream &os, HistoryEvent const &historyEvent)
{
  std::visit (overloaded{ [&os] (auto arg) {
                os << confu_json::type_name<decltype (arg)> ();
                os << ": ";
                os << confu_json::to_json (arg);
              } },
              historyEvent);
  return os;
}

inline std::string
tableAsString (Game const &game)
{
  // TODO sorting does not match with show cards
  // we need to sort and than add the index
  // maybe we skip beeten cards there is no reason that they have an index
  // maybe we put them at the start and the other cards and with an index at the end
  auto result = std::vector<std::tuple<int, Card, boost::optional<Card>>>{};
  pipes::mux (ranges::to<std::vector> (ranges::views::iota (size_t{}, game.getTable ().size ())), game.getTable ()) >>= pipes::transform ([] (auto index, auto &&card) { return std::make_tuple (index, card.first, card.second); }) >>= pipes::push_back (result);
  ranges::sort (result, [] (std::tuple<int, Card, boost::optional<Card>> const &x, std::tuple<int, Card, boost::optional<Card>> const &y) { return std::get<1> (x) < std::get<1> (y); });
  auto cardsMessage = std::stringstream{};
  cardsMessage << "Cards On Table sorted by Value with Index, Value and Type " << std::endl;
  for (auto const &card : result)
    {
      cardsMessage << "index: " << std::get<0> (card) << " " << std::get<1> (card);
      if (std::get<2> (card).has_value ())
        {
          cardsMessage << " " << std::get<2> (card).value () << std::endl;
        }
      else
        {
          cardsMessage << " Card to Beat" << std::endl;
        }
    }
  return cardsMessage.str ();
}

}
#endif /* CB3E895C_E721_4274_A037_FDB32C4DE314 */
