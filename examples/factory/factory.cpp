/*
 * factory.cpp
 *
 *  created on: 26.12.2019
 *      author: kaushik
 */

/*
 * A test example for debugging the negotiation code
 */

#include <array>
#include <iostream>
#include <cmath>
#include <unordered_set>

#include "Component.hpp"
#include "SafetyAutomaton.hpp"
#include "Monitor.hpp"
#include "LivenessGame.hpp"
#include "Spoilers.hpp"
#define _USE_MATH_DEFINES

using namespace std;

/****************************************************************************/
/* main computation */
/****************************************************************************/
int main() {

    negotiation::Component c1("Inputs/C1.txt");
    negotiation::SafetyAutomaton assume;
    assume.readFromFile("Inputs/assume1.txt");
    negotiation::SafetyAutomaton guarantee;
    guarantee.readFromFile("Inputs/guarantee1.txt");
    negotiation::LivenessGame monitor(c1,assume,guarantee);
    
    /* the safety game */
    std::unordered_set<negotiation::abs_type> safe_states = {5};
    std::vector<std::unordered_set<negotiation::abs_type>*> sure_win=monitor.solve_safety_game(safe_states,"sure");
    std::vector<std::unordered_set<negotiation::abs_type>*> maybe_win=monitor.solve_safety_game(safe_states,"maybe");
    
    monitor.find_spoilers(sure_win, maybe_win, "Outputs/spoilers1.txt");
    
    /* read the spoiler automaton */
    negotiation::SafetyAutomaton spoiler_full;
    spoiler_full.readFromFile("Outputs/spoilers1.txt");
    negotiation::Spoilers s1(&spoiler_full);
    s1.boundedBisim(1);
    s1.spoilers_mini_->writeToFile("Outputs/spoilers_mini1.txt");
    s1.spoilers_mini_->determinize();
    s1.spoilers_mini_->writeToFile("Outputs/spoilers_mini_det1.txt");
    
    return 1;
}