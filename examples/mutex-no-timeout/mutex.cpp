/*
 * mutex.cpp
 *
 *  created on: 17.04.2020
 *      author: kaushik
 */

/*
 * A mutual exclusion problem for two processes writing simultaneously on a shared bus
 *  (WITHOUT DEADLINE)
 */

#include <array>
#include <stdio.h>
#include <iostream>
#include <cmath>
#include <unordered_set>
/* for getting the current directory name */
#ifdef WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif


#include "Component.hpp"
#include "SafetyAutomaton.hpp"
#include "Monitor.hpp"
//#include "LivenessGame.hpp"
#include "SafetyGame.hpp"
#include "Spoilers.hpp"
#include "Negotiate.hpp"
#include "TicToc.hpp"
#define _USE_MATH_DEFINES

using namespace std;
using namespace negotiation;

/****************************************************************************/
/* main computation */
/****************************************************************************/
int main() {
    /* maximum depth of negotiation */
    int k_max = 25;

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

    negotiation::Negotiate N(component_files, safe_states_files, target_states_files, k_max);

    TicToc timer;
    /* perform the negotiation */
    timer.tic();
    int k=N.iterative_deepening_search();
    double elapsed_time=timer.toc();

    checkMakeDir("Outputs");
    N.guarantee_[0]->writeToFile("Outputs/guarantee_0.txt");
    N.guarantee_[1]->writeToFile("Outputs/guarantee_1.txt");

    /* save the contracts in DOT language */
    for (int p=0; p<2; p++) {
        std::vector<std::string*> output_labels;
        std::string* str=new std::string;
        *str="wr";
        *str+=std::to_string(p);
        output_labels.push_back(str);
        str=new std::string;
        *str="nop";
        *str+=std::to_string(p);
        output_labels.push_back(str);
        /* save the contract */
        std::string file_op="Outputs/guarantee_";
        file_op+=std::to_string(p);
        file_op+=".gv";
        std::string graph_name="Guarantee_";
        graph_name+=std::to_string(p);
        N.guarantee_[p]->createDOT(file_op, graph_name, output_labels);
    }
    return 1;
}
