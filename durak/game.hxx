#ifndef B3662CAA_D812_46F7_8DD7_C85FCFAC47A4
#define B3662CAA_D812_46F7_8DD7_C85FCFAC47A4

#include "durak/player.hxx"
#include <algorithm>
#include <boost/assign.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <numeric>
#include <pipes/pipes.hpp>
#include <sys/types.h>
#include <vector>

enum struct PlayerRole
{
  attack,
  defend,
  assistAttacker
};

class Game
{
public:
  explicit Game (std::vector<std::string> &&playerNames);

  Game (std::vector<std::string> &&playerNames, std::vector<Card> &&cards);

  bool pass (PlayerRole playerRole);

  void rewokePass (PlayerRole playerRole);

  // attack starts round and can only be used by playr with role attack
  bool playerStartsAttack (std::vector<Card> const &cards);

  // after attack is started player with role attack and assistAttacker can add cards with same value which are allready on the table
  bool playerAssists (PlayerRole playerRole, std::vector<Card> const &cards);

  // defending player can try to beat card on the table
  bool playerDefends (size_t indexFromCardOnTheTable, Card const &card);

  void defendingPlayerTakesAllCardsFromTheTable ();

  std::vector<Player> const &getPlayers () const;

  std::vector<std::pair<Card, std::optional<Card> > > const &getTable () const;

  size_t countOfNotBeatenCardsOnTable () const;

  std::vector<std::pair<size_t, Card> > cardsNotBeatenOnTableWithIndex () const;

  size_t cardsAllowedToPlaceOnTable () const;

  size_t getRound () const;

  Player const &getAttackingPlayer () const;

  Player const &getAssistingPlayer () const;

  Player const &getDefendingPlayer () const;

  Player &getAttackingPlayer ();

  Player &getAssistingPlayer ();

  Player &getDefendingPlayer ();

  bool getAttackStarted () const;

  std::optional<Player> durak () const;

  bool checkIfGameIsOver () const;

  Type getTrump () const;

private:
  void nextRound (bool attackingSuccess);

  std::vector<Card> getTableAsVector ();

  void playerDrawsCardsFromDeck (Player &player, size_t numberOfCards);

  void playerDrawsCardsFromTable (Player &player);

  void calculateNextRoles (bool attackSuccess);

  void drawCards ();

  std::vector<Card> cardDeck{};
  std::vector<Player> players{};
  std::vector<std::pair<Card, std::optional<Card> > > table{};
  Type trump{};
  bool attackStarted = false;
  bool gameOver = false;
  bool attackingPlayerPass = false;
  bool assistingPlayerPass = false;
  size_t round{ 1 };
  size_t numberOfCardsPlayerShouldHave{ 6 };
};

std::vector<Card> generateCardDeck ();

std::optional<Card> drawCard (std::vector<Card> &cardDeck);

std::optional<Card> drawSpecificCard (std::vector<Card> &cardDeck, Card const &cardToDraw);

#endif /* B3662CAA_D812_46F7_8DD7_C85FCFAC47A4 */
