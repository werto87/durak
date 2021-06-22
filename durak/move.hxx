#include "durak/card.hxx"
#include "durak/gameData.hxx"
#include "durak/player.hxx"
#include <map>
#include <variant>
namespace durak
{
struct RoundInformation
{
  std::map<PlayerRole, std::string> playerRoles{};
};

struct StartAttack
{
  std::vector<Card> cards{};
};
struct AssistAttack
{
  PlayerRole playerRole{};
  std::vector<Card> cards{};
};
struct Pass
{
};
struct Defend
{
  Card cardToBeat{};
  Card card{};
};
struct DrawCardsFromTable
{
};

typedef std::variant<RoundInformation, StartAttack, AssistAttack, Pass, Defend, DrawCardsFromTable> History;

}