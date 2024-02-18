#ifndef A85D1720_A39F_43B7_B56C_7843E3A02A0D
#define A85D1720_A39F_43B7_B56C_7843E3A02A0D
#include "durak/card.hxx"
#include <algorithm>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/optional.hpp>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <vector>
namespace durak
{
struct Player
{
public:
  bool
  putCards (std::vector<Card> const &cardsToPut, std::vector<std::pair<Card, boost::optional<Card>>> &target)
  {
    auto cardsToMove = std::stable_partition (cards.begin (), cards.end (), [&cardsToPut] (Card const &card) { return std::find (cardsToPut.begin (), cardsToPut.end (), card) == cardsToPut.end (); });
    if (std::distance (cardsToMove, cards.end ()) != boost::numeric_cast<int> (cardsToPut.size ()))
      {
        return false;
      }
    else
      {
        std::for_each (cardsToMove, cards.end (), [&target] (Card &card) { target.emplace_back (std::pair<Card, boost::optional<Card>>{ card, {} }); });
        cards.erase (cardsToMove, cards.end ());
        return true;
      }
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
    std::ranges::copy_if(cards,std::back_inserter (result),[index=0,&cardIndex](auto const&) mutable {
      auto _result=std::ranges::find (cardIndex.begin (), cardIndex.end (), index) != cardIndex.end ();
      index++;
      return _result;
    });
    if (cardIndex.size () != result.size ()) throw std::logic_error ("cardIndex.size() != result.size()" + std::to_string (cardIndex.size ()) + " != " + std::to_string (result.size ()));
    return result;
  }

  bool
  dropCard (Card const &card)
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

  std::string id{};

private:
  std::vector<Card> cards{};
};

}
#endif /* A85D1720_A39F_43B7_B56C_7843E3A02A0D */
