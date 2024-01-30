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
#include <range/v3/algorithm/find_if.hpp>
#include <range/v3/range.hpp>
#include <stdexcept>
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

TEST_CASE ("play one allowed and than one not allowed card", "[game]")
{
  auto gameOption = GameOption{ .numberOfCardsPlayerShouldHave = 5 };
  gameOption.customCardDeck = testCardDeck8 ();
  auto game = Game{ { "player1", "player2" }, gameOption };
  auto &attackPlayer = game.getAttackingPlayer ().value ();
  auto &defendPlayer = game.getDefendingPlayer ().value ();
  REQUIRE (defendPlayer.getCards ().size () == 3);
  auto cardToPlay = attackPlayer.cardsForIndex ({ 4 });
  REQUIRE (game.playerStartsAttack (cardToPlay));
  REQUIRE (game.playerAssists (PlayerRole::attack, cardToPlay) == false);
}

TEST_CASE ("allowed moves attacking player", "[game]")
{
  auto gameOption = GameOption{};
  gameOption.customCardDeck = testCardDeck8 ();
  auto game = Game{ { "player1", "player2" }, gameOption };
  auto &attackPlayer = game.getAttackingPlayer ().value ();
  auto &defendPlayer = game.getDefendingPlayer ().value ();
  REQUIRE (game.getAllowedMoves (PlayerRole::attack).front () == Move::startAttack);
  game.playerStartsAttack (attackPlayer.cardsForIndex ({ 5 }));
  REQUIRE (game.getAllowedMoves (PlayerRole::attack).front () == Move::addCard);
  game.playerDefends (game.getTable ().front ().first, defendPlayer.getCards ().at (0));
  REQUIRE (game.getAllowedMoves (PlayerRole::attack).size () == 2);
  REQUIRE (std::find_if (game.getAllowedMoves (PlayerRole::attack).begin (), game.getAllowedMoves (PlayerRole::attack).end (), [] (Move allowedMove) { return allowedMove == Move::addCard; }) != game.getAllowedMoves (PlayerRole::attack).end ());
  REQUIRE (std::find_if (game.getAllowedMoves (PlayerRole::attack).begin (), game.getAllowedMoves (PlayerRole::attack).end (), [] (Move allowedMove) { return allowedMove == Move::pass; }) != game.getAllowedMoves (PlayerRole::attack).end ());
}

TEST_CASE ("allowed moves defending player", "[game]")
{
  auto gameOption = GameOption{};
  gameOption.customCardDeck = testCardDeck8 ();
  auto game = Game{ { "player1", "player2" }, gameOption };
  auto &attackPlayer = game.getAttackingPlayer ().value ();
  auto &defendPlayer = game.getDefendingPlayer ().value ();
  REQUIRE (game.getAllowedMoves (PlayerRole::defend).empty ());
  game.playerStartsAttack (attackPlayer.cardsForIndex ({ 5 }));
  REQUIRE (game.getAllowedMoves (PlayerRole::defend).size () == 2);
  REQUIRE (std::find_if (game.getAllowedMoves (PlayerRole::defend).begin (), game.getAllowedMoves (PlayerRole::defend).end (), [] (Move allowedMove) { return allowedMove == Move::defend; }) != game.getAllowedMoves (PlayerRole::defend).end ());
  REQUIRE (std::find_if (game.getAllowedMoves (PlayerRole::defend).begin (), game.getAllowedMoves (PlayerRole::defend).end (), [] (Move allowedMove) { return allowedMove == Move::takeCards; }) != game.getAllowedMoves (PlayerRole::defend).end ());
  game.playerDefends (game.getTable ().front ().first, defendPlayer.getCards ().at (0));
  REQUIRE (game.getAllowedMoves (PlayerRole::defend).size () == 1);
  REQUIRE (std::find_if (game.getAllowedMoves (PlayerRole::defend).begin (), game.getAllowedMoves (PlayerRole::defend).end (), [] (Move allowedMove) { return allowedMove == Move::takeCards; }) != game.getAllowedMoves (PlayerRole::defend).end ());
}

TEST_CASE ("allowed moves assisting player", "[game]")
{
  auto gameOption = GameOption{};
  gameOption.customCardDeck = testCardDeck16 ();
  auto game = Game{ { "player1", "player2", "player3" }, gameOption };
  auto &attackPlayer = game.getAttackingPlayer ().value ();
  auto &defendPlayer = game.getDefendingPlayer ().value ();
  auto &assistingPlayer = game.getAssistingPlayer ().value ();
  REQUIRE (game.getAllowedMoves (PlayerRole::assistAttacker).empty ());
  REQUIRE (game.playerStartsAttack (attackPlayer.cardsForIndex ({ 0 })));
  REQUIRE (game.getAllowedMoves (PlayerRole::assistAttacker).size () == 1);
  REQUIRE (game.getAllowedMoves (PlayerRole::assistAttacker).front () == Move::addCard);
  REQUIRE (game.playerAssists (PlayerRole::assistAttacker, assistingPlayer.cardsForIndex ({ 2 })));
  REQUIRE (game.getAllowedMoves (PlayerRole::assistAttacker).empty ());
  REQUIRE (game.playerDefends (game.getTable ().front ().first, defendPlayer.getCards ().at (0)));
  REQUIRE (game.getAllowedMoves (PlayerRole::assistAttacker).size () == 1);
  REQUIRE (game.playerDefends (game.getTable ().at (1).first, defendPlayer.getCards ().at (0)));
  REQUIRE (game.getAllowedMoves (PlayerRole::assistAttacker).size () == 2);
  // auto history = game.getHistory ();
  // ranges::for_each (history, [] (auto const &historyEvent) { std::cout << historyEvent << std::endl; });
}

TEST_CASE ("game option set trump", "[game]")
{
  auto gameOption = GameOption{};
  gameOption.customCardDeck = testCardDeck16 ();
  auto game = Game{ { "player1", "player2", "player3" }, gameOption };
  REQUIRE (game.getTrump () == Type::hearts);
  gameOption.customCardDeck = testCardDeck16 ();
  gameOption.trump = Type::clubs;
  auto otherGame = Game{ { "player1", "player2", "player3" }, gameOption };
  REQUIRE (game.getTrump () != otherGame.getTrump ());
}

TEST_CASE ("game option set cards for player", "[game]")
{
  auto gameOption = GameOption{};
  gameOption.cardsInHands = std::vector<std::vector<Card>>{};
  auto playerOneCards = std::vector<Card>{ { 1, Type::clubs }, { 1, Type::hearts } };
  auto playerTwoCards = std::vector<Card>{ { 2, Type::clubs }, { 2, Type::hearts } };
  gameOption.cardsInHands->push_back (playerOneCards);
  gameOption.cardsInHands->push_back (playerTwoCards);
  auto game = Game{ { "player1", "player2" }, gameOption };
  REQUIRE (game.getPlayers ().at (0).getCards () == playerOneCards);
  REQUIRE (game.getPlayers ().at (1).getCards () == playerTwoCards);
}
}
