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
//#include "LivenessGame.hpp"
#include "SafetyGame.hpp"
#include "Spoilers.hpp"
#include "Negotiate.hpp"
#define _USE_MATH_DEFINES

using namespace std;

/****************************************************************************/
/* main computation */
/****************************************************************************/
int main() {

    std::vector<std::string*> component_files, safe_states_files, target_states_files;
    std::string* c0= new std::string("Inputs/C1.txt");
    std::string* c1= new std::string("Inputs/C2.txt");
    component_files.push_back(c0);
    component_files.push_back(c1);

    std::string* s0= new std::string("Inputs/safe_states_1.txt");
    std::string* s1= new std::string("Inputs/safe_states_2.txt");
    safe_states_files.push_back(s0);
    safe_states_files.push_back(s1);

    std::string* t0= new std::string("Inputs/target_states_1.txt");
    std::string* t1= new std::string("Inputs/target_states_2.txt");
    target_states_files.push_back(t0);
    target_states_files.push_back(t1);

    // int max_depth=4;
    // negotiation::Negotiate N(component_files, safe_states_files, target_states_files, max_depth);

    negotiation::Negotiate N(component_files, safe_states_files, target_states_files, 25, 2);

    int out_flag = N.fixed_depth_search(3);

    checkMakeDir("Outputs");
    N.guarantee_[0]->writeToFile("Outputs/guarantee_0.txt");
    N.guarantee_[1]->writeToFile("Outputs/guarantee_1.txt");

//    /* testing of the safety part */
//    negotiation::Component c1("Inputs/C1.txt");
//    negotiation::SafetyAutomaton assume(2);
//    negotiation::SafetyAutomaton guarantee(2);
//    negotiation::SafetyGame monitor(c1,assume,guarantee);
//
//    /* the safety game */
//    std::unordered_set<negotiation::abs_type> safe_states = {0,1,2,3,4};
//    std::vector<std::unordered_set<negotiation::abs_type>*> sure_win=monitor.solve_safety_game(safe_states,"sure");
//    std::vector<std::unordered_set<negotiation::abs_type>*> maybe_win=monitor.solve_safety_game(safe_states,"maybe");
//
//    negotiation::SafetyAutomaton* spoiler_full = new negotiation::SafetyAutomaton;
//    monitor.find_spoilers(sure_win, maybe_win, spoiler_full);
//    checkMakeDir("Outputs");
//    spoiler_full->writeToFile("Outputs/spoilers.txt");
//
//    spoiler_full->determinize();
//    spoiler_full->writeToFile("Outputs/spoilers_det.txt");
//
//    negotiation::Spoilers spoilers(spoiler_full);
//    spoilers.boundedBisim(0);
//    spoilers.spoilers_mini_->writeToFile("Outputs/spoilers_mini.txt");
//
//    spoilers.spoilers_mini_->determinize();
//    spoilers.spoilers_mini_->writeToFile("Outputs/spoilers_mini_det.txt");

    return 1;
}
