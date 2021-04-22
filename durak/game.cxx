#include "durak/game.hxx"
#include "durak/card.hxx"
#include "durak/player.hxx"
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <pipes/pipes.hpp>
#include <random>
#include <range/v3/range.hpp>
#include <stdexcept>
namespace durak
{

Game::Game (std::vector<std::string> &&playerNames) : cardDeck{ generateCardDeck () }
{
  trump = cardDeck.front ().type;
  // TODO use move itterator
  std::transform (playerNames.begin (), playerNames.end (), std::back_inserter (players), [] (auto playername) {
    auto player = Player{};
    player.id.swap (playername);
    return player;
  });
  std::for_each (players.begin (), players.end (), [this] (Player &player) { playerDrawsCardsFromDeck (player, numberOfCardsPlayerShouldHave); });
}

Game::Game (std::vector<std::string> &&playerNames, std::vector<Card> &&cards) : cardDeck{ cards }
{
  trump = cardDeck.front ().type;
  // TODO use move itterator
  std::transform (playerNames.begin (), playerNames.end (), std::back_inserter (players), [] (auto playername) {
    auto player = Player{};
    player.id.swap (playername);
    return player;
  });
  std::for_each (players.begin (), players.end (), [this] (Player &player) { playerDrawsCardsFromDeck (player, numberOfCardsPlayerShouldHave); });
}

bool
Game::pass (PlayerRole player)
{
  if ((player == PlayerRole::attack) && attackStarted)
    {
      attackingPlayerPass = true;
    }
  else if (player == PlayerRole::assistAttacker)
    {
      assistingPlayerPass = true;
    }
  if (((players.size () == 2) && attackingPlayerPass && (countOfNotBeatenCardsOnTable () == 0)) || (attackingPlayerPass && assistingPlayerPass && (countOfNotBeatenCardsOnTable () == 0)))
    {
      nextRound (false);
      return true;
    }
  return false;
}

void
Game::rewokePass (PlayerRole player)
{
  if (player == PlayerRole::attack)
    {
      attackingPlayerPass = false;
    }
  else if (player == PlayerRole::assistAttacker)
    {
      assistingPlayerPass = false;
    }
}

bool
Game::playerStartsAttack (std::vector<Card> const &cards)
{
  if (cardsAllowedToPlaceOnTable () >= cards.size ())
    {
      if (cardsHaveSameValue (cards))
        {
          getAttackingPlayer ().putCards (cards, table);
          if (getAttackingPlayer ().getCards ().empty ())
            {
              pass (PlayerRole::attack);
            }
          attackStarted = true;
          return true;
        }
      else
        {
          return false;
        }
    }
  else
    {
      return false;
    }
}

bool
Game::playerAssists (PlayerRole player, std::vector<Card> const &cards)
{
  auto result = false;
  if (player == PlayerRole::attack || player == PlayerRole::assistAttacker)
    {
      // TODO maybe we can do something here to make it more readable
      auto tableVector = getTableAsVector ();
      auto sortByValue = [] (auto const &x, auto const &y) { return x.value > y.value; };
      std::ranges::sort (tableVector, sortByValue);
      auto equal = [] (auto const &x, auto const &y) { return x.value == y.value; };
      tableVector.erase (std::unique (tableVector.begin (), tableVector.end (), equal), tableVector.end ());
      auto isAllowedToPutCards = true;
      for (auto const &card : cards)
        {
          if (not std::ranges::binary_search (tableVector, card, sortByValue))
            {
              isAllowedToPutCards = false;
              break;
            }
        }
      if (isAllowedToPutCards)
        {
          result = true;
          players.at (static_cast<size_t> (player)).putCards (cards, table);
          if (players.at (static_cast<size_t> (player)).getCards ().empty ())
            {
              pass (player);
            }
        }
    }
  return result;
}

bool
Game::playerDefends (size_t indexFromCardOnTheTable, Card const &card)
{
  if (table.size () <= indexFromCardOnTheTable)
    {
      return false;
    }
  auto cardToBeat = table.at (indexFromCardOnTheTable).first;
  if (not table.at (indexFromCardOnTheTable).second && beats (cardToBeat, card, trump))
    {
      table.at (indexFromCardOnTheTable).second = card;
      getDefendingPlayer ().dropCard ({ card });
      if (getDefendingPlayer ().getCards ().empty ())
        {
          nextRound (false);
        }
      else
        {
          rewokePass (PlayerRole::attack);
          rewokePass (PlayerRole::assistAttacker);
        }
      return true;
    }
  else
    {
      return false;
    }
}

void
Game::defendingPlayerTakesAllCardsFromTheTable ()
{
  getDefendingPlayer ().takeCards (getTableAsVector ());
  nextRound (true);
}

void
Game::playerDrawsCardsFromDeck (Player &player, size_t numberOfCards)
{
  auto result = std::vector<Card>{};
  for (size_t i = 0; i < numberOfCards; i++)
    {
      auto card = drawCard (cardDeck);
      if (card)
        {
          result.push_back (card.value ());
        }
      else
        {
          break;
        }
    }
  player.takeCards (std::move (result));
}

void
Game::playerDrawsCardsFromTable (Player &player)
{
  player.takeCards (getTableAsVector ());
}

std::vector<Player> const &
Game::getPlayers () const
{
  return players;
}

std::vector<std::pair<Card, std::optional<Card> > > const &
Game::getTable () const
{
  return table;
}

size_t
Game::countOfNotBeatenCardsOnTable () const
{
  return std::accumulate (table.begin (), table.end (), size_t{ 0 }, [] (auto const &x, std::pair<Card, std::optional<Card> > const &y) { return x + (y.second.has_value () ? 0 : 1); });
}

std::vector<std::pair<size_t, Card> >
Game::cardsNotBeatenOnTableWithIndex () const
{
  auto results = std::vector<std::pair<size_t, Card> >{};
  pipes::mux (ranges::to<std::vector> (std::views::iota (size_t{}, getTable ().size ())), getTable ()) >>= pipes::filter ([] (int, std::pair<Card, std::optional<Card> > b) { return not b.second.has_value (); }) >>= pipes::transform ([] (int a, std::pair<Card, std::optional<Card> > b) { return std::make_pair (a, b.first); }) >>= pipes::push_back (results);
  return results;
}

size_t
Game::cardsAllowedToPlaceOnTable () const
{
  return getDefendingPlayer ().getCards ().size () - countOfNotBeatenCardsOnTable ();
}

std::vector<Card>
Game::getTableAsVector ()
{
  auto result = std::vector<Card>{};
  for (auto &cardPair : table)
    {
      result.push_back (std::move (cardPair.first));
      if (cardPair.second)
        {
          result.push_back (std::move (*cardPair.second));
        }
    }
  return result;
}

void
Game::calculateNextRoles (bool attackSuccess)
{
  if (attackSuccess)
    {
      std::rotate (players.begin (), players.begin () + 1, players.end ());
      std::rotate (players.begin (), players.begin () + 1, players.end ());
    }
  else
    {
      std::rotate (players.begin (), players.begin () + 1, players.end ());
    }
}

void
Game::drawCards ()
{
  if (players.size () >= 3)
    {
      if (getDefendingPlayer ().getCards ().size () < numberOfCardsPlayerShouldHave)
        {
          playerDrawsCardsFromDeck (players.at (static_cast<size_t> (PlayerRole::defend)), numberOfCardsPlayerShouldHave - getDefendingPlayer ().getCards ().size ());
        }
      if (getAssistingPlayer ().getCards ().size () < numberOfCardsPlayerShouldHave)
        {
          playerDrawsCardsFromDeck (players.at (static_cast<size_t> (PlayerRole::assistAttacker)), numberOfCardsPlayerShouldHave - getAssistingPlayer ().getCards ().size ());
        }
      if (getAttackingPlayer ().getCards ().size () < numberOfCardsPlayerShouldHave)
        {
          playerDrawsCardsFromDeck (players.at (static_cast<size_t> (PlayerRole::attack)), numberOfCardsPlayerShouldHave - getAttackingPlayer ().getCards ().size ());
        }
    }
  if (players.size () == 2)
    {
      for (auto &player : players)
        {
          if (player.getCards ().size () < numberOfCardsPlayerShouldHave)
            {
              playerDrawsCardsFromDeck (player, numberOfCardsPlayerShouldHave - player.getCards ().size ());
            }
        }
    }
  players.erase (std::remove_if (players.begin (), players.end (), [] (auto const &player) { return player.getCards ().size () == 0; }), players.end ());
}

void
Game::nextRound (bool attackingSuccess)
{
  std::cout << "NEW ROUND############################################################################################" << std::endl;
  table.clear ();
  round++;
  attackStarted = false;
  rewokePass (PlayerRole::attack);
  rewokePass (PlayerRole::defend);
  drawCards ();
  gameOver = checkIfGameIsOver ();
  calculateNextRoles (attackingSuccess);
}

Player const &
Game::getDefendingPlayer () const
{
  return players.at (static_cast<size_t> (PlayerRole::defend));
}

Player const &
Game::getAssistingPlayer () const
{
  return players.at (static_cast<size_t> (PlayerRole::assistAttacker));
}

Player const &
Game::getAttackingPlayer () const
{
  return players.at (static_cast<size_t> (PlayerRole::attack));
}

Player &
Game::getDefendingPlayer ()
{
  return players.at (static_cast<size_t> (PlayerRole::defend));
}

Player &
Game::getAssistingPlayer ()
{
  return players.at (static_cast<size_t> (PlayerRole::assistAttacker));
}

Player &
Game::getAttackingPlayer ()
{
  return players.at (static_cast<size_t> (PlayerRole::attack));
}

size_t
Game::getRound () const
{
  return round;
}

bool
Game::getAttackStarted () const
{
  return attackStarted;
}

bool
Game::checkIfGameIsOver () const
{
  return players.size () <= 1;
}

std::optional<Player>
Game::durak () const
{
  if (not checkIfGameIsOver ()) throw std::logic_error{ "calling durak and game is not over checkIfGameIsOver () == false" };
  if (players.empty ())
    {
      return {};
    }
  else
    {
      return players.front ();
    }
}

std::vector<Card>
generateCardDeck ()
{
  const size_t cardValueMax = 9;
  std::vector<Card> cardDeck{};
  for (u_int16_t type = 0; type <= 3; type++)
    {
      for (u_int16_t cardValue = 1; cardValue <= cardValueMax; cardValue++)
        {
          cardDeck.push_back (Card{ .value = cardValue, .type = static_cast<Type> (type) });
        }
    }
  static std::random_device rd;
  static std::mt19937 g (rd ());
  std::shuffle (cardDeck.begin (), cardDeck.end (), g);
  return cardDeck;
}

std::optional<Card>
drawCard (std::vector<Card> &cardDeck)
{
  std::optional<Card> card{};
  if (!cardDeck.empty ())
    {
      card = std::move (cardDeck.back ());
      cardDeck.pop_back ();
    }
  return card;
}

std::optional<Card>
drawSpecificCard (std::vector<Card> &cardDeck, Card const &cardToDraw)
{
  auto result = std::optional<Card>{};
  // try to find and if found return it and remove it from cardDeck
  auto card = std::find (cardDeck.begin (), cardDeck.end (), cardToDraw);
  if (card != cardDeck.end ())
    {
      result = std::move (*card);
      cardDeck.erase (card);
    }
  return result;
}

Type
Game::getTrump () const
{
  return trump;
}
}