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
//#include "Negotiate.hpp"
#define _USE_MATH_DEFINES

using namespace std;

/****************************************************************************/
/* main computation */
/****************************************************************************/
int main() {
    
//    std::vector<std::string*> component_files, safe_states_files, target_states_files;
//    std::string* c1= new std::string("C1.txt");
//    std::string* c2= new std::string("C2.txt");
//    component_files.push_back(c1);
//    component_files.push_back(c2);
//
//    std::string* s1= new std::string("safe_states_1.txt");
//    std::string* s2= new std::string("safe_states_2.txt");
//    safe_states_files.push_back(s1);
//    safe_states_files.push_back(s2);
//
//    std::string* t1= new std::string("target_states_1.txt");
//    std::string* t2= new std::string("target_states_2.txt");
//    target_states_files.push_back(t1);
//    target_states_files.push_back(t2);
//
//    int max_depth=2;
//
//    negotiation::Negotiate N(component_files, safe_states_files, target_states_files, max_depth);
//
//    N.iterative_deepening_search();
//
//    N.guarantee_[0]->writeToFile("Outputs/guarantee_1.txt");
//    N.guarantee_[1]->writeToFile("Outputs/guarantee_2.txt");

    negotiation::Component c1("Inputs/C1.txt");
    negotiation::SafetyAutomaton assume(2);
    negotiation::SafetyAutomaton guarantee(2);
    negotiation::SafetyGame monitor(c1,assume,guarantee);

    /* the safety game */
    std::unordered_set<negotiation::abs_type> safe_states = {2,3,4,5,6};
    std::vector<std::unordered_set<negotiation::abs_type>*> sure_win=monitor.solve_safety_game(safe_states,"sure");
    std::vector<std::unordered_set<negotiation::abs_type>*> maybe_win=monitor.solve_safety_game(safe_states,"maybe");

    negotiation::SafetyAutomaton* spoiler_full = new negotiation::SafetyAutomaton;
    monitor.find_spoilers(sure_win, maybe_win, spoiler_full);
    spoiler_full->writeToFile("Outputs/spoilers1.txt");
//
//    /* read the spoiler automaton */
//    negotiation::SafetyAutomaton spoiler_full;
//    spoiler_full.readFromFile("Outputs/spoilers1.txt");
//    negotiation::Spoilers s1(&spoiler_full);
//    s1.boundedBisim(1);
//    s1.spoilers_mini_->writeToFile("Outputs/spoilers_mini1.txt");
//    s1.spoilers_mini_->determinize();
//    s1.spoilers_mini_->writeToFile("Outputs/spoilers_mini_det1.txt");
    
    return 1;
}
