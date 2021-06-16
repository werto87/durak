#include "durak/game.hxx"
#include "durak/card.hxx"
#include "durak/gameData.hxx"
#include "durak/gameOption.hxx"
#include "durak/print.hxx"
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

namespace durak::test
{

TEST_CASE ("game option", "[game]")
{
  auto gameOption = GameOption{};
  gameOption.customCardDeck = testCardDeck ();
  auto game = Game{ { "player1", "player2" }, gameOption };
  auto &player1 = game.getPlayers ().at (0);
  auto &player2 = game.getPlayers ().at (1);
  REQUIRE (player1.getCards ().size () == 6);
  REQUIRE (player1.getCards ().at (0) == Card{ 6, Type::hearts });
  REQUIRE (player2.getCards ().size () == 6);
}

TEST_CASE ("game option player card 7", "[game]")
{
  auto gameOption = GameOption{};
  gameOption.customCardDeck = testCardDeck ();
  gameOption.numberOfCardsPlayerShouldHave = 7;
  auto game = Game{ { "player1", "player2" }, gameOption };
  auto &player1 = game.getPlayers ().at (0);
  auto &player2 = game.getPlayers ().at (1);
  REQUIRE (player1.getCards ().size () == 7);
  REQUIRE (player1.getCards ().at (0) == Card{ 6, Type::hearts });
  REQUIRE (player2.getCards ().size () == 7);
}

TEST_CASE ("its not allowed to play more cards than the defending player has", "[game]")
{
  auto gameOption = GameOption{};
  gameOption.customCardDeck = testCardDeck8 ();
  auto game = Game{ { "player1", "player2" }, gameOption };
  auto &attackPlayer = game.getAttackingPlayer ().value ();
  auto &defendPlayer = game.getDefendingPlayer ().value ();
  REQUIRE (defendPlayer.getCards ().size () == 2);
  game.playerStartsAttack (attackPlayer.cardsForIndex ({ 5 }));
  game.playerAssists (PlayerRole::attack, attackPlayer.cardsForIndex ({ 2, 3, 4 }));
  REQUIRE (game.getTable ().size () <= defendPlayer.getCards ().size ());
}

/*
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
  REQUIRE (game.playerDefends (game.getTable ().at (0).first, game.getPlayers ().at (static_cast<size_t> (PlayerRole::defend)).getCards ().at (3)));
}

TEST_CASE ("playerDefends value to low", "[game]")
{
  auto game = Game{ { "player1", "player2" }, testCardDeck () };
  game.playerStartsAttack ({ { 4, Type::spades }, { 4, Type::clubs } });
  REQUIRE_FALSE (game.playerDefends (game.getTable ().at (0).first, game.getPlayers ().at (static_cast<size_t> (PlayerRole::defend)).getCards ().at (5)));
}

TEST_CASE ("playerDefends player beats all cards", "[game]")
{
  auto game = Game{ { "player1", "player2" }, testCardDeck () };
  game.playerStartsAttack ({ { 4, Type::clubs } });
  REQUIRE (game.countOfNotBeatenCardsOnTable () == 1);
  REQUIRE (game.playerDefends (game.getTable ().at (0).first, game.getPlayers ().at (static_cast<size_t> (PlayerRole::defend)).getCards ().at (3)));
  REQUIRE (game.countOfNotBeatenCardsOnTable () == 0);
}

TEST_CASE ("pass player beats all cards attack and def passes table gets cleared ", "[game]")
{
  auto game = Game{ { "player1", "player2" }, testCardDeck () };
  game.playerStartsAttack ({ { 4, Type::clubs } });
  REQUIRE (game.countOfNotBeatenCardsOnTable () == 1);
  REQUIRE (game.playerDefends (game.getTable ().at (0).first, game.getDefendingPlayer ().value ().getCards ().at (3)));
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
  game.playerDefends (game.getTable ().at (0).first, game.getDefendingPlayer ().value ().getCards ().at (3));
  game.countOfNotBeatenCardsOnTable ();
  game.pass (PlayerRole::attack);
  game.pass (PlayerRole::assistAttacker);
  game.getTable ().size ();
  game.getRound ();
  while (not game.checkIfGameIsOver ())
    {
      game.playerStartsAttack ({ game.getAttackingPlayer ().value ().getCards ().at (0) });
      game.defendingPlayerTakesAllCardsFromTheTable ();
      game.pass (PlayerRole::attack);
      std::cout << game.getRound () << std::endl;
    }
  REQUIRE (game.durak ());
}

TEST_CASE ("getGameData", "[game]")
{
  auto game = Game{ { "player1", "player2" }, testCardDeck () };
  game.playerStartsAttack ({ { 4, Type::clubs } });
  REQUIRE (game.countOfNotBeatenCardsOnTable () == 1);
  REQUIRE (game.playerDefends (game.getTable ().at (0).first, game.getDefendingPlayer ().value ().getCards ().at (3)));
  REQUIRE (game.countOfNotBeatenCardsOnTable () == 0);
  game.pass (PlayerRole::attack);
  game.pass (PlayerRole::assistAttacker);
  REQUIRE (game.getTable ().size () == 0);
  REQUIRE (game.getRound () == 2);
  auto gameData = game.getGameData ();
  REQUIRE (gameData.players.size () == game.getPlayers ().size ());
  REQUIRE (gameData.table.size () == game.getTable ().size ());
  REQUIRE (gameData.trump == game.getTrump ());
}

TEST_CASE ("playground", "[game]")
{

  auto playerData = PlayerData{};
  std::cout << confu_boost::toString (playerData) << std::endl;
  auto gameData = GameData{};
  std::cout << confu_boost::toString (gameData) << std::endl;
}
*/
}
