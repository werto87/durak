#include "durak/player.hxx"
#include "durak/game.hxx"
#include <algorithm>
#include <array>
#include <catch2/catch.hpp>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <sys/types.h>

namespace durak::test
{

TEST_CASE ("smallest card value can not beat a card", "[beats]")
{
  auto cardDeck = generateCardDeck ();
  auto cardToCompare = drawSpecificCard (cardDeck, Card{ .value = 1, .type = Type::clubs });
  auto cardsBeaten = 0;
  while (auto card = drawCard (cardDeck))
    {
      if (beats (card.value (), cardToCompare.value (), Type::diamonds))
        {
          cardsBeaten++;
        }
    }
  REQUIRE (cardsBeaten == 0);
  REQUIRE (cardDeck.empty ());
}

TEST_CASE ("smallest card gets beaten by  cards with same symbol or trump", "[beats]")
{
  auto cardDeck = generateCardDeck ();
  auto cardToCompare = drawSpecificCard (cardDeck, Card{ .value = 1, .type = Type::clubs });
  auto cardsBeaten = 0;
  while (auto card = drawCard (cardDeck))
    {
      if (beats (cardToCompare.value (), card.value (), Type::diamonds))
        {
          cardsBeaten++;
        }
    }
  REQUIRE (cardsBeaten == 17);
  REQUIRE (cardDeck.empty ());
}

TEST_CASE ("highest card value and not trump beats every card with same symbol", "[beats]")
{
  auto cardDeck = generateCardDeck ();
  auto cardToCompare = drawSpecificCard (cardDeck, Card{ .value = 9, .type = Type::clubs });
  auto cardsBeaten = 0;
  while (auto card = drawCard (cardDeck))
    {
      if (beats (card.value (), cardToCompare.value (), Type::diamonds))
        {
          cardsBeaten++;
        }
    }
  REQUIRE (cardsBeaten == 8);
  REQUIRE (cardDeck.empty ());
}

TEST_CASE ("highest card value and not trump gets only beaten by trump", "[beats]")
{
  auto cardDeck = generateCardDeck ();
  auto cardToCompare = drawSpecificCard (cardDeck, Card{ .value = 9, .type = Type::clubs });
  auto cardsBeaten = 0;
  while (auto card = drawCard (cardDeck))
    {
      if (beats (cardToCompare.value (), card.value (), Type::diamonds))
        {
          cardsBeaten++;
        }
    }
  REQUIRE (cardsBeaten == 9);
  REQUIRE (cardDeck.empty ());
}

TEST_CASE ("highest card and trump beats every card", "[beats]")
{
  auto cardDeck = generateCardDeck ();
  auto cardToCompare = drawSpecificCard (cardDeck, Card{ .value = 9, .type = Type::clubs });
  auto cardsBeaten = 0;
  while (auto card = drawCard (cardDeck))
    {
      if (beats (card.value (), cardToCompare.value (), Type::clubs))
        {
          cardsBeaten++;
        }
    }
  REQUIRE (cardsBeaten == 35);
  REQUIRE (cardDeck.empty ());
}

TEST_CASE ("highest card and trump cant be beaten", "[beats]")
{
  auto cardDeck = generateCardDeck ();
  auto cardToCompare = drawSpecificCard (cardDeck, Card{ .value = 9, .type = Type::clubs });
  auto cardsBeaten = 0;
  while (auto card = drawCard (cardDeck))
    {
      if (beats (cardToCompare.value (), card.value (), Type::clubs))
        {
          cardsBeaten++;
        }
    }
  REQUIRE (cardsBeaten == 0);
  REQUIRE (cardDeck.empty ());
}

TEST_CASE ("lowest trump card and  beats every card which is not a trump", "[beats]")
{
  auto cardDeck = generateCardDeck ();
  auto cardToCompare = drawSpecificCard (cardDeck, Card{ .value = 1, .type = Type::clubs });
  auto cardsBeaten = 0;
  while (auto card = drawCard (cardDeck))
    {
      if (beats (card.value (), cardToCompare.value (), Type::clubs))
        {
          cardsBeaten++;
        }
    }
  REQUIRE (cardsBeaten == 27);
  REQUIRE (cardDeck.empty ());
}

TEST_CASE ("lowest card and trump gets only beaten by other trump cards", "[beats]")
{
  auto cardDeck = generateCardDeck ();
  auto cardToCompare = drawSpecificCard (cardDeck, Card{ .value = 1, .type = Type::clubs });
  auto cardsBeaten = 0;
  while (auto card = drawCard (cardDeck))
    {
      if (beats (cardToCompare.value (), card.value (), Type::clubs))
        {
          cardsBeaten++;
        }
    }
  REQUIRE (cardsBeaten == 8);
  REQUIRE (cardDeck.empty ());
}

TEST_CASE ("cardsForIndex", "[cardsForIndex]")
{
  auto player = Player{};
  player.takeCards ({ { .value = 1, .type = Type::clubs }, { .value = 1, .type = Type::diamonds }, { .value = 2, .type = Type::clubs }, { .value = 2, .type = Type::diamonds }, { .value = 3, .type = Type::spades }, { .value = 4, .type = Type::clubs } });
  auto cards = player.cardsForIndex ({ 0, 1, 2 });
  REQUIRE (cards == std::vector<Card>{ { .value = 1, .type = Type::clubs }, { .value = 1, .type = Type::diamonds }, { .value = 2, .type = Type::clubs } });
}

TEST_CASE ("cardsForIndex out of bound", "[cardsForIndex]")
{
  auto player = Player{};
  player.takeCards ({ { .value = 1, .type = Type::clubs }, { .value = 1, .type = Type::diamonds }, { .value = 2, .type = Type::clubs }, { .value = 2, .type = Type::diamonds }, { .value = 3, .type = Type::spades }, { .value = 4, .type = Type::clubs } });
  REQUIRE_THROWS (player.cardsForIndex ({ 41, 1, 2 }));
}

}
