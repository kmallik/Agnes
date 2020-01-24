/*
 * test.cpp
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
#include "SafetyGame.hpp"
#include "Spoilers.hpp"
#define _USE_MATH_DEFINES

using namespace std;

/****************************************************************************/
/* main computation */
/****************************************************************************/
int main() {

    negotiation::Component c("Inputs/C.txt");
    negotiation::SafetyAutomaton assume;
    assume.readFromFile("Inputs/assume.txt");
    negotiation::SafetyAutomaton guarantee;
    guarantee.readFromFile("Inputs/guarantee.txt");

    /* the safety game */
    std::unordered_set<negotiation::abs_type> safe_states = {0, 1, 2, 3, 4};
//    /* allowed inputs */
//    std::vector<std::unordered_set<negotiation::abs_type>*> allowed_inputs;
//    for (negotiation::abs_type i=0; i<c.no_states; i++) {
//        std::unordered_set<negotiation::abs_type>* s = new std::unordered_set<negotiation::abs_type>;
//        s->insert(0);
//        allowed_inputs.push_back(s);
//    }

    negotiation::SafetyGame monitor(c,assume,guarantee);
    monitor.writeToFile("monitor.txt");
    std::vector<std::unordered_set<negotiation::abs_type>*> sure_win=monitor.solve_safety_game(safe_states,"sure");
    std::vector<std::unordered_set<negotiation::abs_type>*> maybe_win=monitor.solve_safety_game(safe_states,"maybe");

//    negotiation::SafetyAutomaton* spoiler_full = new negotiation::SafetyAutomaton;
//    bool out=monitor.find_spoilers(spoiler_full);
//
//    /* read the spoiler automaton */
////    negotiation::SafetyAutomaton spoiler_full;
////    spoiler_full.readFromFile("Outputs/spoilers1.txt");
//    negotiation::Spoilers s1(spoiler_full);
//    s1.boundedBisim(1);
//    checkMakeDir("Outputs");
//    s1.spoilers_mini_->writeToFile("Outputs/spoilers_mini1.txt");
//    s1.spoilers_mini_->determinize();
//    s1.spoilers_mini_->writeToFile("Outputs/spoilers_mini_det1.txt");

    return 1;
}
