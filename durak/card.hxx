#ifndef AE375AF6_DDFE_4D22_AD80_723EB970B8C7
#define AE375AF6_DDFE_4D22_AD80_723EB970B8C7

#include <algorithm>
#include <cstddef>
#include <optional>
#include <random>
#include <range/v3/all.hpp>
#include <sys/types.h>
#include <vector>
namespace durak
{
enum struct Type
{
  hearts,
  clubs,
  diamonds,
  spades
};

struct Card
{
  // Regular
  friend bool operator== (Card const &x, Card const &y);

  friend bool operator!= (Card const &x, Card const &y);

  // TotallyOrdered
  friend bool
  operator< (const Card &x, const Card &y)
  {
    if (x.value < y.value) return true;
    if (y.value < x.value) return false;
    if (x.type < y.type) return true;
    if (y.type < x.type) return false;
    return false;
  }
  friend bool
  operator> (const Card &x, const Card &y)
  {
    return y < x;
  }
  friend bool
  operator<= (const Card &x, const Card &y)
  {
    return !(y < x);
  }
  friend bool
  operator>= (const Card &x, const Card &y)
  {
    return !(x < y);
  }

  u_int16_t value{};
  Type type{};
};

inline bool beats (Card const &cardToBeat, Card const &cardWhichTriesTobeat, Type trump);

inline bool hasSameValue (Card const &x, Card const &y);

inline bool cardsHaveSameValue (std::vector<Card> const &cards);

inline bool
operator== (Card const &x, Card const &y)
{
  return (x.value == y.value) && (x.type == y.type);
}

inline bool
operator!= (Card const &x, Card const &y)
{
  return !(x == y);
}

inline bool
cardsHaveSameValue (std::vector<Card> const &cards)
{
  return std::find_if (cards.begin (), cards.end (), [valueToCompare = cards.front ().value] (Card const &card) { return valueToCompare != card.value; }) == cards.end ();
}

inline bool
hasSameValue (Card const &x, Card const &y)
{
  return x.value == y.value;
}

inline bool
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
}
#endif /* AE375AF6_DDFE_4D22_AD80_723EB970B8C7 */