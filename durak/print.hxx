#ifndef CB3E895C_E721_4274_A037_FDB32C4DE314
#define CB3E895C_E721_4274_A037_FDB32C4DE314
#include <iostream>
#include <vector>
struct Card;
struct Player;
class Game;

std::ostream &operator<< (std::ostream &os, Card const &card);

std::ostream &operator<< (std::ostream &os, Player const &player);

std::string cardsSortedByValueWithIndex (std::vector<Card> cards);

std::string attackingPlayerWithNameAndCardIndexValueAndType (Game const &game);

std::string defendingPlayerWithNameAndCardIndexValueAndType (Game const &game);

std::string assistingPlayerWithNameAndCardIndexValueAndType (Game const &game);

std::string tableAsString (Game const &game);
#endif /* CB3E895C_E721_4274_A037_FDB32C4DE314 */
