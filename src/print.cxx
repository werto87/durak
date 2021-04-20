#include "src/print.hxx"
#include "src/game.hxx"
#include <fmt/format.h>
#include <magic_enum.hpp>
#include <pipes/pipes.hpp>
#include <range/v3/range.hpp>
#include <ranges>
#include <sstream>
std::ostream &
operator<< (std::ostream &os, Card const &card)
{
  os << fmt::format ("Card: {{{0} , {1}}}", card.value, magic_enum::enum_name (card.type));
  return os;
}

std::ostream &
operator<< (std::ostream &os, Player const &player)
{
  os << "Player Cards sorted by Value" << std::endl;
  auto cards = player.getCards ();
  std::ranges::sort (cards);
  for (auto const &card : cards)
    os << card << std::endl;
  return os;
}

std::string
cardsSortedByValueWithIndex (std::vector<Card> cards)
{
  std::ranges::sort (cards);
  auto cardsMessage = std::stringstream{};
  cardsMessage << "Cards sorted by Value with Index, Value and Type " << std::endl;
  for (size_t i = 0; auto const &card : cards)
    {
      i++;
      cardsMessage << "index: " << i << " " << card << std::endl;
    }

  return cardsMessage.str ();
}

std::string
attackingPlayerWithNameAndCardIndexValueAndType (Game const &game)
{
  auto result = std::stringstream{};
  result << "Attacking Player: " << game.getAttackingPlayer ().id << std::endl << "trump: " << magic_enum::enum_name (game.getTrump ()) << std::endl << cardsSortedByValueWithIndex (game.getAttackingPlayer ().getCards ());
  return result.str ();
}

std::string
defendingPlayerWithNameAndCardIndexValueAndType (Game const &game)
{
  auto result = std::stringstream{};
  result << "Defending Player: " << game.getDefendingPlayer ().id << std::endl << "trump: " << magic_enum::enum_name (game.getTrump ()) << std::endl << cardsSortedByValueWithIndex (game.getDefendingPlayer ().getCards ());
  return result.str ();
}

std::string
assistingPlayerWithNameAndCardIndexValueAndType (Game const &game)
{
  auto result = std::stringstream{};
  result << "Assisting Player: " << game.getAssistingPlayer ().id << std::endl << "trump: " << magic_enum::enum_name (game.getTrump ()) << std::endl << cardsSortedByValueWithIndex (game.getAssistingPlayer ().getCards ());
  return result.str ();
}

std::string
tableAsString (Game const &game)
{
  auto result = std::vector<std::tuple<int, Card, std::optional<Card> > >{};
  pipes::mux (ranges::to<std::vector> (std::views::iota (size_t{}, game.getTable ().size ())), game.getTable ()) >>= pipes::transform ([] (auto index, auto &&card) { return std::make_tuple (index, card.first, card.second); }) >>= pipes::push_back (result);
  std::ranges::sort (result, [] (std::tuple<int, Card, std::optional<Card> > const &x, std::tuple<int, Card, std::optional<Card> > const &y) { return std::get<1> (x) < std::get<1> (y); });
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