#include "durak/card.hxx"
#include <algorithm>
#include <cstddef>
#include <pipes/pipes.hpp>
#include <pipes/push_back.hpp>
#include <pipes/transform.hpp>
#include <random>
#include <range/v3/all.hpp>

namespace durak
{
bool
operator== (Card const &x, Card const &y)
{
  return (x.value == y.value) && (x.type == y.type);
}
bool
operator!= (Card const &x, Card const &y)
{
  return !(x == y);
}

bool
beats (Card const &cardToBeat, Card const &cardWhichTriesTobeat, Type trump)
{
  if (cardToBeat.type == cardWhichTriesTobeat.type && cardToBeat.value < cardWhichTriesTobeat.value)
    {
      return true;
    }
  else if ((cardToBeat.type != trump) && (cardWhichTriesTobeat.type == trump))
    {
      return true;
    }
  else
    {
      return false;
    }
}

bool
hasSameValue (Card const &x, Card const &y)
{
  return x.value == y.value;
}

bool
cardsHaveSameValue (std::vector<Card> const &cards)
{
  return std::find_if (cards.begin (), cards.end (), [valueToCompare = cards.front ().value] (Card const &card) { return valueToCompare != card.value; }) == cards.end ();
}

std::vector<Card>
sortedCardIndexing (std::vector<Card> cards, std::vector<size_t> const &indexes)
{
  auto result = std::vector<Card>{};
  ranges::sort (cards);
  pipes::mux (ranges::to_vector (ranges::views::iota (size_t{ 1 }, cards.size () + 1)), cards) >>= pipes::filter ([&indexes] (size_t i, Card) { return indexes.end () != ranges::find (indexes, i); }) >>= pipes::transform ([] (size_t, Card const &card) { return card; }) >>= pipes::push_back (result);
  return result;
}
}