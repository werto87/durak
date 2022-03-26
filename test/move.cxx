#include "durak/move.hxx"
#include "durak/card.hxx"
#include <catch2/catch.hpp>

using namespace durak;

TEST_CASE ("compare history event", "[]")
{
  auto event1 = HistoryEvent{ RoundInformation{} };
  auto event2 = HistoryEvent{ StartAttack{} };
  REQUIRE (event1 != event2);
  auto event3 = HistoryEvent{ StartAttack{} };
  auto event4 = HistoryEvent{ StartAttack{} };
  std::get<1> (event3).cards = { { 1, Type::clubs } };
  std::get<1> (event4).cards = { { 1, Type::clubs } };
  REQUIRE (event3 == event4);
  std::get<1> (event4).cards = { { 2, Type::clubs } };
  REQUIRE (event3 != event4);
}