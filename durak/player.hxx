#ifndef A85D1720_A39F_43B7_B56C_7843E3A02A0D
#define A85D1720_A39F_43B7_B56C_7843E3A02A0D
#include "durak/card.hxx"
#include <algorithm>
#include <boost/optional.hpp>
#include <cstddef>
#include <pipes/dev_null.hpp>
#include <pipes/pipes.hpp>
#include <range/v3/all.hpp>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <vector>
namespace durak
{
struct Player
{
public:
  void
  putCards (std::vector<Card> const &cardsToPut, std::vector<std::pair<Card, boost::optional<Card>>> &target)
  {
    auto cardsToMove = std::stable_partition (cards.begin (), cards.end (), [&cardsToPut] (Card const &card) { return std::find (cardsToPut.begin (), cardsToPut.end (), card) == cardsToPut.end (); });
    std::for_each (cardsToMove, cards.end (), [&target] (Card &card) { target.emplace_back (std::pair<Card, boost::optional<Card>>{ std::move (card), {} }); });
    cards.erase (cardsToMove, cards.end ());
  }

  void
  takeCards (std::vector<Card> &&cardsToTake)
  {
    std::move (cardsToTake.begin (), cardsToTake.end (), std::back_inserter (cards));
    cardsToTake.clear ();
  }

  std::vector<Card> const &
  getCards () const
  {
    return cards;
  }

  std::vector<Card>
  cardsForIndex (std::vector<size_t> const &cardIndex) const
  {
    if (cardIndex.size () > cards.size ()) throw std::logic_error ("cardIndex.size() > cards.size ()" + std::to_string (cardIndex.size ()) + " != " + std::to_string (cards.size ()));
    auto result = std::vector<Card>{};
    pipes::mux (ranges::to<std::vector> (ranges::views::iota (size_t{}, cards.size ())), cards) >>= pipes::filter ([&cardIndex] (int i, auto) { return std::find (cardIndex.begin (), cardIndex.end (), i) != cardIndex.end (); }) >>= pipes::transform ([] (auto, auto &&card) { return card; }) >>= pipes::push_back (result);
    if (cardIndex.size () != result.size ()) throw std::logic_error ("cardIndex.size() != result.size()" + std::to_string (cardIndex.size ()) + " != " + std::to_string (result.size ()));
    return result;
  }

  bool
  dropCard (Card const &card)
  {
    auto cardItr = ranges::find (cards, card);
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

  std::string id{};

private:
  std::vector<Card> cards{};
};

}
#endif /* A85D1720_A39F_43B7_B56C_7843E3A02A0D */
