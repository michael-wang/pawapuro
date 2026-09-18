#pragma once
#include <string>
#include <stdexcept>
namespace pawapuro {
// Startup Data for the one active development batter; no gameplay consumers yet.
struct BatterProfile {
    std::string display_name;
    int contact=0,power=0,trajectory=1;
};
inline char ability_grade(int value) {
    if(value<0||value>120)throw std::out_of_range("Ability must be in [0, 120].");
    if(value<50)return 'F';
    if(value<60)return 'E';
    if(value<70)return 'D';
    if(value<80)return 'C';
    if(value<90)return 'B';
    if(value<100)return 'A';
    return 'S';
}
}
