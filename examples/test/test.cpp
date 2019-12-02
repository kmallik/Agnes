/*
 * test.cc
 *
 *  created on: 15.11.2019
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
#define _USE_MATH_DEFINES

using namespace std;

/****************************************************************************/
/* main computation */
/****************************************************************************/
int main() {

    negotiation::Component c1("Inputs/C1.txt");
    negotiation::SafetyAutomaton assume("Inputs/assume.txt");
    negotiation::SafetyAutomaton guarantee("Inputs/guarantee.txt");
    negotiation::Monitor monitor(c1,assume,guarantee);
    
    /* the safety game */
    std::unordered_set<negotiation::abs_type> safe_states = {0,1,2,3,4};
    std::vector<std::unordered_set<negotiation::abs_type>*> win_dom=monitor.solve_safety_game(safe_states,"maybe");
//    for (int i=0; i<win_dom.size(); i++) {
//        delete win_dom[i];
//    }
    
    return 1;
}
