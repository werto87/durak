#include "src/game.hxx"
#include "src/card.hxx"
#include "src/print.hxx"
#include "test/constant.hxx"
#include <algorithm>
#include <array>
#include <catch2/catch.hpp>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <pipes/pipes.hpp>
#include <range/v3/range.hpp>
#include <sys/types.h>
#include <tuple>
#include <utility>
#include <vector>
namespace test
{

TEST_CASE ("at the start of the game the players have 6 cards", "[game]")
{
  auto game = Game{ { "player1", "player2" }, testCardDeck () };
  auto &player1 = game.getPlayers ().at (0);
  auto &player2 = game.getPlayers ().at (1);
  REQUIRE (player1.getCards ().size () == 6);
  REQUIRE (player2.getCards ().size () == 6);
}

TEST_CASE ("player starts attack", "[game]")
{
  auto game = Game{ { "player1", "player2" }, testCardDeck () };
  REQUIRE (game.getPlayers ().at (static_cast<size_t> (PlayerRole::attack)).getCards ().size () == 6);

  REQUIRE (game.playerStartsAttack ({ { 4, Type::diamonds }, { 4, Type::spades }, { 4, Type::clubs } }));
  REQUIRE (game.getPlayers ().at (static_cast<size_t> (PlayerRole::attack)).getCards ().size () == 3);
}

TEST_CASE ("player assists attack", "[game]")
{
  auto game = Game{ { "player1", "player2" }, testCardDeck () };
  auto cards = game.getPlayers ().at (static_cast<size_t> (PlayerRole::attack)).getCards ();
  REQUIRE (cards.size () == 6);
  game.playerStartsAttack ({ { 4, Type::spades }, { 4, Type::clubs } });
  cards = game.getPlayers ().at (static_cast<size_t> (PlayerRole::attack)).getCards ();
  REQUIRE (cards.size () == 4);
  REQUIRE (game.playerAssists (PlayerRole::attack, { { 4, Type::diamonds }, { 4, Type::hearts } }));
  cards = game.getPlayers ().at (static_cast<size_t> (PlayerRole::attack)).getCards ();
  REQUIRE (cards.size () == 2);
  REQUIRE (game.getTable ().size () == 4);
}

TEST_CASE ("cardsNotBeatenOnTable empty table", "[game]")
{
  auto game = Game{ { "player1", "player2" }, testCardDeck () };
  REQUIRE (game.countOfNotBeatenCardsOnTable () == 0);
}

TEST_CASE ("countOfNotBeatenCardsOnTable table with two cards which are not beaten", "[game]")
{
  auto game = Game{ { "player1", "player2" }, testCardDeck () };
  game.playerStartsAttack ({ { 4, Type::spades }, { 4, Type::clubs } });
  REQUIRE (game.countOfNotBeatenCardsOnTable () == 2);
}

TEST_CASE ("cardsNotBeatenOnTableWithIndex table with two cards which are not beaten", "[game]")
{
  auto game = Game{ { "player1", "player2" }, testCardDeck () };
  game.playerStartsAttack ({ { 4, Type::spades }, { 4, Type::clubs } });
  REQUIRE (game.cardsNotBeatenOnTableWithIndex ().size () == 2);
  auto [cardIndex, card] = game.cardsNotBeatenOnTableWithIndex ().at (0);
  REQUIRE (cardIndex == 0);
  REQUIRE (card.value == 4);
  REQUIRE (card.type == Type::clubs);
}

TEST_CASE ("playerDefends player beats one card of two cards on the table", "[game]")
{
  auto game = Game{ { "player1", "player2" }, testCardDeck () };
  game.playerStartsAttack ({ { 4, Type::spades }, { 4, Type::clubs } });
  REQUIRE (game.playerDefends (0, game.getPlayers ().at (static_cast<size_t> (PlayerRole::defend)).getCards ().at (3)));
}

TEST_CASE ("playerDefends value to low", "[game]")
{
  auto game = Game{ { "player1", "player2" }, testCardDeck () };
  game.playerStartsAttack ({ { 4, Type::spades }, { 4, Type::clubs } });
  REQUIRE_FALSE (game.playerDefends (0, game.getPlayers ().at (static_cast<size_t> (PlayerRole::defend)).getCards ().at (5)));
}

TEST_CASE ("playerDefends player beats all cards", "[game]")
{
  auto game = Game{ { "player1", "player2" }, testCardDeck () };
  game.playerStartsAttack ({ { 4, Type::clubs } });
  REQUIRE (game.countOfNotBeatenCardsOnTable () == 1);
  REQUIRE (game.playerDefends (0, game.getPlayers ().at (static_cast<size_t> (PlayerRole::defend)).getCards ().at (3)));
  REQUIRE (game.countOfNotBeatenCardsOnTable () == 0);
}

TEST_CASE ("pass player beats all cards attack and def passes table gets cleared ", "[game]")
{
  auto game = Game{ { "player1", "player2" }, testCardDeck () };
  game.playerStartsAttack ({ { 4, Type::clubs } });
  REQUIRE (game.countOfNotBeatenCardsOnTable () == 1);
  REQUIRE (game.playerDefends (0, game.getDefendingPlayer ().getCards ().at (3)));
  REQUIRE (game.countOfNotBeatenCardsOnTable () == 0);
  game.pass (PlayerRole::attack);
  game.pass (PlayerRole::assistAttacker);
  REQUIRE (game.getTable ().size () == 0);
  REQUIRE (game.getRound () == 2);
}

TEST_CASE ("try to play the game", "[game]")
{
  auto game = Game{ { "player1", "player2" }, testCardDeck () };
  game.playerStartsAttack ({ { 4, Type::clubs } });
  game.countOfNotBeatenCardsOnTable ();
  game.playerDefends (0, game.getDefendingPlayer ().getCards ().at (3));
  game.countOfNotBeatenCardsOnTable ();
  game.pass (PlayerRole::attack);
  game.pass (PlayerRole::assistAttacker);
  game.getTable ().size ();
  game.getRound ();
  while (not game.checkIfGameIsOver ())
    {

      game.playerStartsAttack ({ game.getAttackingPlayer ().getCards ().at (0) });
      game.defendingPlayerTakesAllCardsFromTheTable ();
    }
  REQUIRE (game.durak ());
}

TEST_CASE ("playground", "[game]") {}

}
