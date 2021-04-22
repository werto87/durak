#include "durak/player.hxx"
#include <pipes/dev_null.hpp>
#include <pipes/pipes.hpp>
#include <range/v3/range.hpp>
#include <stdexcept>
#include <string>
namespace durak
{
void
Player::putCards (std::vector<Card> const &cardsToPut, std::vector<std::pair<Card, std::optional<Card> > > &target)
{
  auto cardsToMove = std::stable_partition (cards.begin (), cards.end (), [&cardsToPut] (Card const &card) { return std::find (cardsToPut.begin (), cardsToPut.end (), card) == cardsToPut.end (); });
  std::for_each (cardsToMove, cards.end (), [&target] (Card &card) { target.emplace_back (std::pair<Card, std::optional<Card> >{ std::move (card), {} }); });
  cards.erase (cardsToMove, cards.end ());
}

void
Player::takeCards (std::vector<Card> &&cardsToTake)
{
  std::move (cardsToTake.begin (), cardsToTake.end (), std::back_inserter (cards));
  cardsToTake.clear ();
}

std::vector<Card> const &
Player::getCards () const
{
  return cards;
}

std::vector<Card>
Player::cardsForIndex (std::vector<size_t> const &cardIndex) const
{
  if (cardIndex.size () > cards.size ()) throw std::logic_error ("cardIndex.size() > cards.size ()" + std::to_string (cardIndex.size ()) + " != " + std::to_string (cards.size ()));
  auto result = std::vector<Card>{};
  pipes::mux (ranges::to<std::vector> (std::views::iota (size_t{}, cards.size ())), cards) >>= pipes::filter ([&cardIndex] (int i, auto) { return std::find (cardIndex.begin (), cardIndex.end (), i) != cardIndex.end (); }) >>= pipes::transform ([] (auto, auto &&card) { return card; }) >>= pipes::push_back (result);
  if (cardIndex.size () != result.size ()) throw std::logic_error ("cardIndex.size() != result.size()" + std::to_string (cardIndex.size ()) + " != " + std::to_string (result.size ()));
  return result;
}

bool
Player::dropCard (Card const &card)
{
  auto cardItr = std::ranges::find (cards, card);
  if (cardItr != cards.end ())
    {
      cards.erase (cardItr);
      return true;
    }
  else
    {
      return false;
    }
}
}