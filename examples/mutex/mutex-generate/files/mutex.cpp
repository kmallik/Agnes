/*
 * mutex.cpp
 *
 *  created on: 25.01.2020
 *      author: kaushik
 */

/*
 * A mutual exclusion problem for two processes writing simultaneously on a shared bus
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
    std::string* c1= new std::string("Inputs/pr_0.txt");
    std::string* c2= new std::string("Inputs/pr_1.txt");
    component_files.push_back(c1);
    component_files.push_back(c2);

    std::string* s1= new std::string("Inputs/safe_states_0.txt");
    std::string* s2= new std::string("Inputs/safe_states_1.txt");
    safe_states_files.push_back(s1);
    safe_states_files.push_back(s2);

    std::string* t1= new std::string("Inputs/target_states_0.txt");
    std::string* t2= new std::string("Inputs/target_states_1.txt");
    target_states_files.push_back(t1);
    target_states_files.push_back(t2);

    negotiation::Negotiate N(component_files, safe_states_files, target_states_files);

    N.iterative_deepening_search();

    checkMakeDir("Outputs");
    N.guarantee_[0]->writeToFile("Outputs/guarantee_0.txt");
    N.guarantee_[1]->writeToFile("Outputs/guarantee_1.txt");

    return 1;
}
