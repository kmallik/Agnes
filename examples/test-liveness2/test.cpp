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
#include "LivenessGame.hpp"
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

    /* the liveness game */
    std::unordered_set<negotiation::abs_type> target_states = {1};
    /* allowed inputs */
    std::vector<std::unordered_set<negotiation::abs_type>*> allowed_inputs, allowed_joint_inputs;
    negotiation::abs_type no_monitor_states=c.no_states*(assume.no_states_-1)*(guarantee.no_states_-1)+2;
    for (negotiation::abs_type i=0; i<no_monitor_states; i++) {
        std::unordered_set<negotiation::abs_type>* s = new std::unordered_set<negotiation::abs_type>;
        s->insert(0);
        s->insert(1);
        allowed_inputs.push_back(s);
        std::unordered_set<negotiation::abs_type>* sj = new std::unordered_set<negotiation::abs_type>;
        sj->insert(0);
        sj->insert(1);
        sj->insert(2);
        sj->insert(3);
        allowed_joint_inputs.push_back(sj);
    }

    negotiation::LivenessGame monitor(c,assume,guarantee,target_states,allowed_inputs, allowed_joint_inputs);
    std::vector<std::unordered_set<negotiation::abs_type>*> sure_win=monitor.solve_liveness_game("sure");
    std::vector<std::unordered_set<negotiation::abs_type>*> maybe_win=monitor.solve_liveness_game("maybe");

    negotiation::SafetyAutomaton* spoiler_full = new negotiation::SafetyAutomaton;
    bool out=monitor.find_spoilers(spoiler_full);

    /* read the spoiler automaton */
//    negotiation::SafetyAutomaton spoiler_full;
//    spoiler_full.readFromFile("Outputs/spoilers1.txt");
    negotiation::Spoilers s1(spoiler_full);
    s1.boundedBisim(1);
    checkMakeDir("Outputs");
    s1.spoilers_mini_->writeToFile("Outputs/spoilers_mini1.txt");
    s1.spoilers_mini_->determinize();
    s1.spoilers_mini_->writeToFile("Outputs/spoilers_mini_det1.txt");

    return 1;
}
