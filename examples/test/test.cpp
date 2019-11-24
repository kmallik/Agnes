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
    
    return 1;
}
