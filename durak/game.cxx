#include "durak/game.hxx"
#include "durak/card.hxx"
#include "durak/gameData.hxx"
#include "durak/player.hxx"
#include <algorithm>
#include <bits/ranges_algo.h>
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
Game::playerStartsAttack (std::vector<Card> const &cards)
{
  if (auto attackingPlayer = getAttackingPlayer ())
    {
      if (cardsAllowedToPlaceOnTable () >= cards.size ())
        {
          if (cardsHaveSameValue (cards))
            {
              attackingPlayer.value ().putCards (cards, table);
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
        }
    }
  return result;
}

bool
Game::playerDefends (Card const &cardToBeat, Card const &card)
{
  if (auto defendingPlayer = getDefendingPlayer ())
    {
      if (auto cardToBeatItr = std::ranges::find_if (table, [&cardToBeat] (auto const &cardAndOptionalCard) { return cardAndOptionalCard.first == cardToBeat; }); cardToBeatItr != table.end () && not cardToBeatItr->second && beats (cardToBeatItr->first, card, trump))
        {
          if (defendingPlayer.value ().dropCard (card))
            {
              cardToBeatItr->second = card;
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
Game::defendingPlayerTakesAllCardsFromTheTable ()
{
  if (auto defendingPlayer = getDefendingPlayer (); defendingPlayer && not table.empty ())
    {
      defendingPlayer.value ().takeCards (getTableAsVector ());
      return true;
    }
  else
    {
      return false;
    }
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

std::vector<std::pair<Card, boost::optional<Card>>> const &
Game::getTable () const
{
  return table;
}

size_t
Game::countOfNotBeatenCardsOnTable () const
{
  return std::accumulate (table.begin (), table.end (), size_t{ 0 }, [] (auto const &x, std::pair<Card, boost::optional<Card>> const &y) { return x + (y.second.has_value () ? 0 : 1); });
}

std::vector<std::pair<size_t, Card>>
Game::cardsNotBeatenOnTableWithIndex () const
{
  auto results = std::vector<std::pair<size_t, Card>>{};
  pipes::mux (ranges::to<std::vector> (std::views::iota (size_t{}, getTable ().size ())), getTable ()) >>= pipes::filter ([] (int, std::pair<Card, boost::optional<Card>> b) { return not b.second.has_value (); }) >>= pipes::transform ([] (int a, std::pair<Card, boost::optional<Card>> b) { return std::make_pair (a, b.first); }) >>= pipes::push_back (results);
  return results;
}

size_t
Game::cardsAllowedToPlaceOnTable () const
{
  auto defendingPlayer = getDefendingPlayer ();
  return defendingPlayer ? defendingPlayer.value ().getCards ().size () - countOfNotBeatenCardsOnTable () : 0;
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
  // TODO get player functions returns optional we can use this instead off size
  if (players.size () >= 3)
    {
      if (getDefendingPlayer ().value ().getCards ().size () < numberOfCardsPlayerShouldHave)
        {
          playerDrawsCardsFromDeck (players.at (static_cast<size_t> (PlayerRole::defend)), numberOfCardsPlayerShouldHave - getDefendingPlayer ().value ().getCards ().size ());
        }
      if (getAssistingPlayer ().value ().getCards ().size () < numberOfCardsPlayerShouldHave)
        {
          playerDrawsCardsFromDeck (players.at (static_cast<size_t> (PlayerRole::assistAttacker)), numberOfCardsPlayerShouldHave - getAssistingPlayer ().value ().getCards ().size ());
        }
      if (getAttackingPlayer ().value ().getCards ().size () < numberOfCardsPlayerShouldHave)
        {
          playerDrawsCardsFromDeck (players.at (static_cast<size_t> (PlayerRole::attack)), numberOfCardsPlayerShouldHave - getAttackingPlayer ().value ().getCards ().size ());
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
  if (attackingSuccess) defendingPlayerTakesAllCardsFromTheTable ();
  table.clear ();
  round++;
  attackStarted = false;
  drawCards ();
  gameOver = checkIfGameIsOver ();
  calculateNextRoles (attackingSuccess);
}
boost::optional<const Player &>
Game::getDefendingPlayer () const
{
  if (players.size () > static_cast<size_t> (PlayerRole::defend))
    {
      return players.at (static_cast<size_t> (PlayerRole::defend));
    }
  else
    {
      return {};
    }
}

boost::optional<const Player &>
Game::getAssistingPlayer () const
{
  if (players.size () > static_cast<size_t> (PlayerRole::assistAttacker))
    {
      return players.at (static_cast<size_t> (PlayerRole::assistAttacker));
    }
  else
    {
      return {};
    }
}

boost::optional<const Player &>
Game::getAttackingPlayer () const
{
  if (players.size () > static_cast<size_t> (PlayerRole::attack))
    {
      return players.at (static_cast<size_t> (PlayerRole::attack));
    }
  else
    {
      return {};
    }
}

boost::optional<Player &>
Game::getDefendingPlayer ()
{
  if (players.size () > static_cast<size_t> (PlayerRole::defend))
    {
      return players.at (static_cast<size_t> (PlayerRole::defend));
    }
  else
    {
      return {};
    }
}

boost::optional<Player &>
Game::getAssistingPlayer ()
{
  if (players.size () > static_cast<size_t> (PlayerRole::assistAttacker))
    {
      return players.at (static_cast<size_t> (PlayerRole::assistAttacker));
    }
  else
    {
      return {};
    }
}

boost::optional<Player &>
Game::getAttackingPlayer ()
{
  if (players.size () > static_cast<size_t> (PlayerRole::attack))
    {
      return players.at (static_cast<size_t> (PlayerRole::attack));
    }
  else
    {
      return {};
    }
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

boost::optional<Player>
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

boost::optional<Card>
drawCard (std::vector<Card> &cardDeck)
{
  boost::optional<Card> card{};
  if (!cardDeck.empty ())
    {
      card = std::move (cardDeck.back ());
      cardDeck.pop_back ();
    }
  return card;
}

boost::optional<Card>
drawSpecificCard (std::vector<Card> &cardDeck, Card const &cardToDraw)
{
  auto result = boost::optional<Card>{};
  // try to find and if found return it and remove it from cardDeck
  auto card = std::find (cardDeck.begin (), cardDeck.end (), cardToDraw);
  if (card != cardDeck.end ())
    {
      result = std::move (*card);
      cardDeck.erase (card);
    }
  return result;
}

GameData
Game::getGameData () const
{
  auto result = GameData{};
  result.trump = trump;
  result.table = table;
  auto assistingPlayer = getAssistingPlayer ();
  auto attackingPlayer = getAttackingPlayer ();
  auto defendingPlayer = getDefendingPlayer ();
  std::ranges::transform (players, std::back_inserter (result.players), [&] (Player const &player) {
    auto playerRole = PlayerRole::waiting;
    if (assistingPlayer && player.id == assistingPlayer.value ().id)
      {
        playerRole = PlayerRole::assistAttacker;
      }
    else if (attackingPlayer && player.id == attackingPlayer.value ().id)
      {
        playerRole = PlayerRole::attack;
      }
    else if (defendingPlayer && player.id == defendingPlayer.value ().id)
      {
        playerRole = PlayerRole::defend;
      }
    auto playerData = PlayerData{};
    playerData.name = player.id;
    // playerData.cards = player.getCards ();
    std::ranges::transform (player.getCards (), std::back_inserter (playerData.cards), [] (durak::Card const &card) { return card; });
    playerData.playerRole = playerRole;
    return playerData;
  });
  result.round = getRound ();
  return result;
}

PlayerRole
Game::getRoleForName (std::string const &name) const
{
  auto defendingPlayer = getDefendingPlayer ();
  auto attackingPlayer = getAttackingPlayer ();
  auto assistingPlayer = getAssistingPlayer ();
  if (defendingPlayer && defendingPlayer->id == name)
    {
      return PlayerRole::defend;
    }
  else if (attackingPlayer && attackingPlayer->id == name)
    {
      return PlayerRole::attack;
    }
  else if (assistingPlayer && assistingPlayer->id == name)
    {
      return PlayerRole::assistAttacker;
    }
  else
    {
      return PlayerRole::waiting;
    }
}

Type
Game::getTrump () const
{
  return trump;
}
}