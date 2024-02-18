#ifndef CB3E895C_E721_4274_A037_FDB32C4DE314
#define CB3E895C_E721_4274_A037_FDB32C4DE314
#include "durak/game.hxx"
#include <confu_json/confu_json.hxx>
#include <fmt/format.h>
#include <iostream>
#include <magic_enum/magic_enum.hpp>
#include <sstream>
#include <vector>

namespace durak
{
// helper type for the visitor #4
template <class... Ts> struct overloaded : Ts...
{
  using Ts::operator()...;
};
// explicit deduction guide (not needed as of C++20)
template <class... Ts> overloaded (Ts...) -> overloaded<Ts...>;

struct Card;
struct Player;

inline std::ostream &operator<< (std::ostream &os, Card const &card);

inline std::ostream &operator<< (std::ostream &os, Player const &player);

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
  std::ranges::sort (cards);
  for (auto const &card : cards)
    os << card << std::endl;
  return os;
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



}
#endif /* CB3E895C_E721_4274_A037_FDB32C4DE314 */
