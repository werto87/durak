#ifndef B3662CAA_D812_46F7_8DD7_C85FCFAC47A4
#define B3662CAA_D812_46F7_8DD7_C85FCFAC47A4

#include "durak/card.hxx"
#include "durak/constant.hxx"
#include "durak/gameData.hxx"
#include "durak/gameOption.hxx"
#include "durak/move.hxx"
#include "durak/player.hxx"
#include <algorithm>
#include <boost/assign.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <numeric>
#include <pipes/pipes.hpp>
#include <random>
#include <range/v3/algorithm/find_if.hpp>
#include <range/v3/all.hpp>
#include <range/v3/view/filter.hpp>
#include <stdexcept>
#include <sys/types.h>
#include <vector>
namespace durak
{

enum struct Move
{
  startAttack,
  addCard,
  pass,
  defend,
  takeCards
};

enum struct ResultType
{
  cardsGoToGraveyard,
  cardsGoToPlayer
};
struct RoundResult
{
  ResultType resultType{};
  std::vector<Card> cards{};
  size_t deckSize{};
};

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
  auto card = std::find (cardDeck.begin (), cardDeck.end (), cardToDraw);
  if (card != cardDeck.end ())
    {
      result = std::move (*card);
      cardDeck.erase (card);
    }
  return result;
}

struct GameState
{
  std::vector<Card> cardDeck{};
  std::vector<Player> players{};
  std::vector<std::pair<Card, boost::optional<Card>>> table{};
  Type trump{};
  bool attackStarted = false;
  bool gameOver = false;
  size_t round{ defaultRoundToStart };
  size_t numberOfCardsPlayerShouldHave{ defaultNumberOfCardsPlayerShouldHave };
};

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

    trump = gameOption.trump ? gameOption.trump.value () : (not cardDeck.empty ()) ? cardDeck.front ().type : Type::hearts;
    // TODO use move itterator
    std::transform (playerNames.begin (), playerNames.end (), std::back_inserter (players), [] (auto playername) {
      auto player = Player{};
      player.id.swap (playername);
      return player;
    });
    if (gameOption.cardsInHands)
      {
        std::for_each (players.begin (), players.end (), [index = size_t{ 0 }, cardsInHands = gameOption.cardsInHands] (Player &player) mutable { player.takeCards (std::move (cardsInHands->at (index++))); });
      }
    else
      {
        std::for_each (players.begin (), players.end (), [this] (Player &player) { playerDrawsCardsFromDeck (player, numberOfCardsPlayerShouldHave); });
      }
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

  Game (GameState gameState) : cardDeck{ gameState.cardDeck }, players{ gameState.players }, table{ gameState.table }, trump{ gameState.trump }, attackStarted{ gameState.attackStarted }, gameOver{ gameState.gameOver }, round{ gameState.round }, numberOfCardsPlayerShouldHave{ gameState.numberOfCardsPlayerShouldHave } {}

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
                auto startAttack = StartAttack{};
                startAttack.cards = cards;
                history.push_back (startAttack);
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
    if (cardsAllowedToPlaceOnTable () >= cards.size ())
      {
        if (playerRole == PlayerRole::attack || playerRole == PlayerRole::assistAttacker)
          {
            auto tableVector = getTableAsVector ();
            ranges::sort (tableVector);
            for (auto i = size_t{}; i != cards.size (); ++i)
              {
                if (ranges::binary_search (tableVector, cards.at (i)))
                  {
                    return false;
                  }
              }
            auto tableValues = std::vector<decltype (Card::value)>{};
            ranges::transform (tableVector, ranges::back_inserter (tableValues), [] (auto const &card) { return card.value; });
            for (auto i = size_t{}; i != cards.size (); ++i)
              {
                if (not ranges::binary_search (tableValues, cards.at (i).value))
                  {
                    return false;
                  }
              }
            if (players.at (static_cast<size_t> (playerRole)).putCards (cards, table))
              {
                auto assistAttack = AssistAttack{};
                assistAttack.cards = cards;
                assistAttack.playerRole = playerRole;
                history.push_back (assistAttack);
                return true;
              }
            else
              {
                return false;
              }
          }
      }
    return false;
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
            auto defend = Defend{};
            defend.cardToBeat = cardToBeat;
            defend.card = card;
            history.push_back (defend);
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
    if (cardDeck.empty () && boost::numeric_cast<unsigned long> (ranges::count_if (players, [] (Player const &player) { return player.getCards ().empty (); })) == players.size ())
      {
        return {};
      }
    else
      {
        return players.front ();
      }
  }

  bool
  checkIfGameIsOver () const
  {
    return gameOver || players.size () <= 1 || (cardDeck.empty () && boost::numeric_cast<unsigned long> (ranges::count_if (players, [] (Player const &player) { return player.getCards ().empty (); })) == players.size ());
  }

  RoundResult
  nextRound (bool attackingSuccess)
  {
    if (attackingSuccess)
      {
        history.push_back (DrawCardsFromTable{});
      }
    else
      {
        history.push_back (Pass{});
      }
    auto roundResult = RoundResult{};
    roundResult.cards = getTableAsVector ();
    roundResult.resultType = attackingSuccess ? ResultType::cardsGoToPlayer : ResultType::cardsGoToGraveyard;
    if (attackingSuccess) defendingPlayerTakesAllCardsFromTheTable ();
    table.clear ();
    round++;
    attackStarted = false;
    drawCards ();
    gameOver = checkIfGameIsOver ();
    calculateNextRoles (attackingSuccess);
    roundResult.deckSize = cardDeck.size ();
    auto roundInformation = RoundInformation{};
    if (auto attackingPlayer = getAttackingPlayer ()) roundInformation.playerRoles.push_back ({ PlayerRole::attack, attackingPlayer.value ().id });
    if (auto defendingPlayer = getDefendingPlayer ()) roundInformation.playerRoles.push_back ({ PlayerRole::defend, defendingPlayer.value ().id });
    if (auto assistingPlayer = getAssistingPlayer ()) roundInformation.playerRoles.push_back ({ PlayerRole::assistAttacker, assistingPlayer.value ().id });
    history.push_back (roundInformation);
    return roundResult;
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
  getTableAsVector () const
  {
    auto result = std::vector<Card>{};
    for (auto cardPair : table)
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
    if (players.size () > 1)
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
      std::transform (player.getCards ().begin (), player.getCards ().end (), std::back_inserter (playerData.cards), [] (durak::Card const &card) { return card; });
      playerData.playerRole = playerRole;
      return playerData;
    });
    result.round = getRound ();
    result.cardsInDeck = cardDeck.size ();
    if (not cardDeck.empty ()) result.lastCardInDeck = cardDeck.front ();
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

  bool
  hasCardWhichIsAllowedToAdd (PlayerRole playerRole) const
  {
    if (playerRole == PlayerRole::attack || playerRole == PlayerRole::assistAttacker)
      {
        auto cardsOnTable = getTableAsVector ();
        auto playerCards = std::vector<Card>{};
        if (PlayerRole::attack == playerRole)
          {
            if (auto attackingPlayer = getAttackingPlayer ())
              {
                playerCards = attackingPlayer->getCards ();
              }
          }
        else
          {
            if (auto assistingPlayer = getAssistingPlayer ())
              {
                playerCards = assistingPlayer->getCards ();
              }
          }
        for (auto const &card : getTableAsVector ())
          {
            if (ranges::find_if (playerCards, [&card] (Card const &_card) { return card.value == _card.value; }) != playerCards.end ())
              {
                return true;
              }
          }
        return false;
      }
    else
      {
        return false;
      }
  }

  bool
  hasCardWhichIsAllowedDefend (PlayerRole playerRole) const
  {
    if (playerRole == PlayerRole::defend)
      {
        if (auto defendingPlayer = getDefendingPlayer ())
          {
            auto playerCards = defendingPlayer->getCards ();
            for (auto const &cardPair : getTable () | ranges::views::filter ([] (auto const &cardPair) { return !cardPair.second.has_value (); }))
              {
                if (ranges::find_if (playerCards, [&card = cardPair.first, &trump = trump] (Card const &_card) { return beats (card, _card, trump); }) != playerCards.end ())
                  {
                    return true;
                  }
              }
          }
        return false;
      }
    else
      {
        return false;
      }
  }

  bool
  isAllowedToStartAttack (PlayerRole playerRole) const
  {
    return cardsAllowedToPlaceOnTable () >= 1 && (not attackStarted and playerRole == PlayerRole::attack and table.empty () and getAttackingPlayer () and not getAttackingPlayer ().value ().getCards ().empty () and getDefendingPlayer () and not getDefendingPlayer ().value ().getCards ().empty ());
  }
  bool
  isAllowedToAddCard (PlayerRole playerRole) const
  {
    return cardsAllowedToPlaceOnTable () >= 1 && ((playerRole == PlayerRole::attack || playerRole == PlayerRole::assistAttacker) and attackStarted and hasCardWhichIsAllowedToAdd (playerRole));
  }
  bool
  isAllowedToPass (PlayerRole playerRole) const
  {
    return attackStarted and (playerRole == PlayerRole::attack || playerRole == PlayerRole::assistAttacker) and countOfNotBeatenCardsOnTable () == 0;
  }
  bool
  isAllowedToDefend (PlayerRole playerRole) const
  {
    return playerRole == PlayerRole::defend and countOfNotBeatenCardsOnTable () > 0 and hasCardWhichIsAllowedDefend (playerRole);
  }
  bool
  isAllowedToTakeCards (PlayerRole playerRole) const
  {
    return playerRole == PlayerRole::defend and not table.empty ();
  }

  std::vector<Move>
  getAllowedMoves (PlayerRole playerRole) const
  {
    // TODO if two players attack it make sense to wait (do nothing for a couple of seconds maybe other player adds card)
    auto allowedMoves = std::vector<Move>{};
    if (isAllowedToStartAttack (playerRole)) allowedMoves.push_back (Move::startAttack);
    if (isAllowedToAddCard (playerRole)) allowedMoves.push_back (Move::addCard);
    if (isAllowedToPass (playerRole)) allowedMoves.push_back (Move::pass);
    if (isAllowedToDefend (playerRole)) allowedMoves.push_back (Move::defend);
    if (isAllowedToTakeCards (playerRole)) allowedMoves.push_back (Move::takeCards);
    return allowedMoves;
  }

  std::vector<HistoryEvent> const &
  getHistory () const
  {
    return history;
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
  std::vector<HistoryEvent> history{};
};
}
#endif /* B3662CAA_D812_46F7_8DD7_C85FCFAC47A4 */
