#include "src/game.hxx"
#include "src/card.hxx"
#include "src/player.hxx"
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <pipes/pipes.hpp>
#include <random>
#include <range/v3/range.hpp>

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

Game::Game (size_t playerCount) : cardDeck{ generateCardDeck () }, players (playerCount)
{
  trump = cardDeck.back ().type;
  std::for_each (players.begin (), players.end (), [this] (Player &player) { playerDrawsCardsFromDeck (player, numberOfCardsPlayerShouldHave); });
}

Game::Game (size_t playerCount, std::vector<Card> &&cards) : cardDeck{ cards }, players (playerCount)
{
  trump = cardDeck.back ().type;
  std::for_each (players.begin (), players.end (), [this] (Player &player) { playerDrawsCardsFromDeck (player, numberOfCardsPlayerShouldHave); });
}

void
Game::pass (PlayerRole player)
{
  if (player == PlayerRole::attack)
    {
      attackingPlayerPass = true;
    }
  else if (player == PlayerRole::assistAttacker)
    {
      assistingPlayerPass = true;
    }
  if (attackingPlayerPass && assistingPlayerPass && getAttackingPlayer ().getCards ().empty ())
    {
      nextRound (false);
    }
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
Game::playerStartsAttack (std::vector<size_t> const &index)
{
  if (cardsAllowedToPlaceOnTable () >= index.size ())
    {
      auto cards = getDefendingPlayer ().cardsForIndex (index);
      if (cardsHaveSameValue (cards))
        {
          getDefendingPlayer ().putCards (cards, table);
          if (getDefendingPlayer ().getCards ().empty ())
            {
              pass (PlayerRole::attack);
            }
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
Game::playerAssists (PlayerRole player, std::vector<size_t> const &index)
{
  auto cards = getDefendingPlayer ().cardsForIndex (index);
  auto result = false;
  if (player == PlayerRole::attack || player == PlayerRole::assistAttacker)
    {
      auto tableVector = getTableAsVector ();
      auto sortByValue = [] (auto const &x, auto const &y) { return x.value > y.value; };
      std::sort (tableVector.begin (), tableVector.end (), sortByValue);
      tableVector.erase (std::unique (tableVector.begin (), tableVector.end (), sortByValue), tableVector.end ());
      auto isAllowedToPutCards = true;
      for (auto const &card : cards)
        {
          if (not std::binary_search (tableVector.begin (), tableVector.end (), card, sortByValue))
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
  auto cardToBeat = table.at (indexFromCardOnTheTable).first;
  if (not table.at (indexFromCardOnTheTable).second && beats (cardToBeat, card, trump))
    {
      table.at (indexFromCardOnTheTable).second = card;
      getAttackingPlayer ().putCards ({ card }, table);
      if (getAttackingPlayer ().getCards ().empty ())
        {
          nextRound (false);
        }
      else
        {
          rewokePass (PlayerRole::attack);
          rewokePass (PlayerRole::defend);
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
  getAttackingPlayer ().takeCards (getTableAsVector ());
  table.clear ();
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
  return getAttackingPlayer ().getCards ().size () - countOfNotBeatenCardsOnTable ();
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
  players.erase (std::remove_if (players.begin (), players.end (), [] (Player const &player) { return player.getCards ().empty (); }), players.end ());
  if (attackSuccess)
    {
      std::rotate (players.begin (), players.begin () + 1, players.end ());
    }
  else
    {
      std::rotate (players.begin (), players.begin () + 2, players.end ());
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
}

void
Game::nextRound (bool attackingSuccess)
{
  round++;
  rewokePass (PlayerRole::attack);
  rewokePass (PlayerRole::defend);
  drawCards ();
  calculateNextRoles (attackingSuccess);
}

Player
Game::getDefendingPlayer () const
{
  return players.at (static_cast<size_t> (PlayerRole::attack));
}

Player
Game::getAssistingPlayer () const
{
  return players.at (static_cast<size_t> (PlayerRole::assistAttacker));
}

Player
Game::getAttackingPlayer () const
{
  return players.at (static_cast<size_t> (PlayerRole::defend));
}

size_t
Game::getRound ()
{
  return round;
}
