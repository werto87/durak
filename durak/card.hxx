#ifndef AE375AF6_DDFE_4D22_AD80_723EB970B8C7
#define AE375AF6_DDFE_4D22_AD80_723EB970B8C7

#include <cstddef>
#include <optional>
#include <sys/types.h>
#include <vector>
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

bool beats (Card const &cardToBeat, Card const &cardWhichTriesTobeat, Type trump);

bool hasSameValue (Card const &x, Card const &y);

bool cardsHaveSameValue (std::vector<Card> const &cards);

// takes an unsorted cards vector sorts it and returns a vector with cards for the indexes
std::vector<Card> sortedCardIndexing (std::vector<Card> cards, std::vector<size_t> const &indexes);

#endif /* AE375AF6_DDFE_4D22_AD80_723EB970B8C7 */
