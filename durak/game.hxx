#ifndef B3662CAA_D812_46F7_8DD7_C85FCFAC47A4
#define B3662CAA_D812_46F7_8DD7_C85FCFAC47A4

#include "durak/card.hxx"
#include "durak/constant.hxx"
#include "durak/gameData.hxx"
#include "durak/gameOption.hxx"
#include "durak/player.hxx"
#include <algorithm>
#include <boost/assign.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <numeric>
#include <pipes/pipes.hpp>
#include <random>
#include <range/v3/all.hpp>
#include <stdexcept>
#include <sys/types.h>
#include <vector>
namespace durak
{

inline std::vector<Card>
generateCardDeck (u_int16_t maxValue = defaultMaxCardValue, u_int16_t typeCount = defaultTypeCount)
{
  std::vector<Card> cardDeck{};
  for (u_int16_t type = 0; type < typeCount; type++)
    {
      for (u_int16_t cardValue = 1; cardValue <= maxValue; cardValue++)
        {
          cardDeck.push_back (Card{ .value = cardValue, .type = static_cast<Type> (type) });
        }
    }
  static std::random_device rd;
  static std::mt19937 g (rd ());
  std::shuffle (cardDeck.begin (), cardDeck.end (), g);
  return cardDeck;
}

inline boost::optional<Card>
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

inline boost::optional<Card>
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

class Game
{
public:
  Game () = default;
  explicit Game (std::vector<std::string> &&playerNames) : cardDeck{ generateCardDeck () }
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

  Game (std::vector<std::string> &&playerNames, GameOption gameOption) : round (gameOption.roundToStart), numberOfCardsPlayerShouldHave{ gameOption.numberOfCardsPlayerShouldHave }
  {
    if (gameOption.customCardDeck)
      {
        cardDeck = gameOption.customCardDeck.value ();
      }
    else
      {
        cardDeck = generateCardDeck (gameOption.maxCardValue, gameOption.typeCount);
      }
    trump = cardDeck.front ().type;
    // TODO use move itterator
    std::transform (playerNames.begin (), playerNames.end (), std::back_inserter (players), [] (auto playername) {
      auto player = Player{};
      player.id.swap (playername);
      return player;
    });
    std::for_each (players.begin (), players.end (), [this] (Player &player) { playerDrawsCardsFromDeck (player, numberOfCardsPlayerShouldHave); });
  }

  Game (std::vector<std::string> &&playerNames, std::vector<Card> &&cards) : cardDeck{ cards }
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

  // attack starts round and can only be used by playr with role attack
  bool
  playerStartsAttack (std::vector<Card> const &cards)
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

  // after attack is started player with role attack and assistAttacker can add cards with same value which are allready on the table
  bool
  playerAssists (PlayerRole playerRole, std::vector<Card> const &cards)
  {
    auto result = false;
    if (playerRole == PlayerRole::attack || playerRole == PlayerRole::assistAttacker)
      {
        // TODO maybe we can do something here to make it more readable
        auto tableVector = getTableAsVector ();
        auto sortByValue = [] (auto const &x, auto const &y) { return x.value > y.value; };
        ranges::sort (tableVector, sortByValue);
        auto equal = [] (auto const &x, auto const &y) { return x.value == y.value; };
        tableVector.erase (std::unique (tableVector.begin (), tableVector.end (), equal), tableVector.end ());
        auto isAllowedToPutCards = true;
        for (auto const &card : cards)
          {
            if (not ranges::binary_search (tableVector, card, sortByValue))
              {
                isAllowedToPutCards = false;
                break;
              }
          }
        if (isAllowedToPutCards)
          {
            result = true;
            players.at (static_cast<size_t> (playerRole)).putCards (cards, table);
          }
      }
    return result;
  }

  // defending player can try to beat card on the table
  bool
  playerDefends (Card const &cardToBeat, Card const &card)
  {
    if (auto defendingPlayer = getDefendingPlayer ())
      {
        if (auto cardToBeatItr = ranges::find_if (table, [&cardToBeat] (auto const &cardAndOptionalCard) { return cardAndOptionalCard.first == cardToBeat; }); cardToBeatItr != table.end () && not cardToBeatItr->second && beats (cardToBeatItr->first, card, trump))
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

  std::vector<Player> const &
  getPlayers () const
  {
    return players;
  }

  std::vector<std::pair<Card, boost::optional<Card>>> const &
  getTable () const
  {
    return table;
  }

  size_t
  countOfNotBeatenCardsOnTable () const
  {
    return std::accumulate (table.begin (), table.end (), size_t{ 0 }, [] (auto const &x, std::pair<Card, boost::optional<Card>> const &y) { return x + (y.second.has_value () ? 0 : 1); });
  }

  std::vector<std::pair<size_t, Card>>
  cardsNotBeatenOnTableWithIndex () const
  {
    auto results = std::vector<std::pair<size_t, Card>>{};
    pipes::mux (ranges::to<std::vector> (ranges::views::iota (size_t{}, getTable ().size ())), getTable ()) >>= pipes::filter ([] (int, std::pair<Card, boost::optional<Card>> b) { return not b.second.has_value (); }) >>= pipes::transform ([] (int a, std::pair<Card, boost::optional<Card>> b) { return std::make_pair (a, b.first); }) >>= pipes::push_back (results);
    return results;
  }

  size_t
  cardsAllowedToPlaceOnTable () const
  {
    auto defendingPlayer = getDefendingPlayer ();
    return defendingPlayer ? defendingPlayer.value ().getCards ().size () - countOfNotBeatenCardsOnTable () : 0;
  }

  size_t
  getRound () const
  {
    return round;
  }

  boost::optional<const Player &>
  getAttackingPlayer () const
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

  boost::optional<const Player &>
  getAssistingPlayer () const
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
  getDefendingPlayer () const
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
  getAttackingPlayer ()
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
  getAssistingPlayer ()
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
  getDefendingPlayer ()
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

  bool
  getAttackStarted () const
  {
    return attackStarted;
  }

  // empty optional means no player lost
  boost::optional<Player>
  durak () const
  {
    if (not checkIfGameIsOver ()) throw std::logic_error{ "calling durak and game is not over checkIfGameIsOver () == false" };
    if (cardDeck.empty () && ranges::count_if (players, [] (Player const &player) { return player.getCards ().empty (); }) == players.size ())
      {
        return {};
      }
    else if (players.size () == 1)
      {
        return players.front ();
      }
    else
      {
        throw std::logic_error{ "error while calculating loser in  durak::Game::durak ()" };
      }
  }

  bool
  checkIfGameIsOver () const
  {
    return players.size () <= 1 || (cardDeck.empty () && ranges::count_if (players, [] (Player const &player) { return player.getCards ().empty (); }) == players.size ());
  }

  void
  nextRound (bool attackingSuccess)
  {
    if (attackingSuccess) defendingPlayerTakesAllCardsFromTheTable ();
    table.clear ();
    round++;
    attackStarted = false;
    drawCards ();
    gameOver = checkIfGameIsOver ();
    calculateNextRoles (attackingSuccess);
  }

  bool
  defendingPlayerTakesAllCardsFromTheTable ()
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

  std::vector<Card>
  getTableAsVector ()
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
  playerDrawsCardsFromDeck (Player &player, size_t numberOfCards)
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
  playerDrawsCardsFromTable (Player &player)
  {
    player.takeCards (getTableAsVector ());
  }

  void
  calculateNextRoles (bool attackSuccess)
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
  drawCards ()
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

  GameData
  getGameData () const
  {
    auto result = GameData{};
    result.trump = trump;
    result.table = table;
    auto assistingPlayer = getAssistingPlayer ();
    auto attackingPlayer = getAttackingPlayer ();
    auto defendingPlayer = getDefendingPlayer ();

    std::transform (players.begin (), players.end (), std::back_inserter (result.players), [&] (Player const &player) {
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
      std::transform (player.getCards ().begin (), player.getCards ().end (), std::back_inserter (playerData.cards), [] (durak::Card const &card) { return card; });
      playerData.playerRole = playerRole;
      return playerData;
    });
    result.round = getRound ();
    return result;
  }

  PlayerRole
  getRoleForName (std::string const &name) const
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
  getTrump () const
  {
    return trump;
  }

  void
  removePlayer (std::string const &accountName)
  {
    if (auto playerItr = std::find_if (players.begin (), players.end (), [&accountName] (Player const &_player) { return _player.id == accountName; }); playerItr != players.end ())
      {
        gameOver = true;
        std::iter_swap (playerItr, players.begin ());
      }
  }

private:
  std::vector<Card> cardDeck{};
  std::vector<Player> players{};
  std::vector<std::pair<Card, boost::optional<Card>>> table{};
  Type trump{};
  bool attackStarted = false;
  bool gameOver = false;
  size_t round{ defaultRoundToStart };
  size_t numberOfCardsPlayerShouldHave{ defaultNumberOfCardsPlayerShouldHave };
};

}
#endif /* B3662CAA_D812_46F7_8DD7_C85FCFAC47A4 */
