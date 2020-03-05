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
#include "TicToc.hpp"
#define _USE_MATH_DEFINES

using namespace std;
using namespace negotiation;

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

    /* save the transition systems in DOT language */
    for (int p=0; p<2; p++) {
        std::vector<std::string*> state_labels;
        abs_type no_state_clusters;
        std::vector<std::unordered_set<abs_type>*> state_clusters;
        std::string file_ip="Inputs/pr_";
        file_ip+=std::to_string(p);
        file_ip+=".txt";
        readVec<std::string>(file_ip, state_labels, (size_t)N.components_[p]->no_states, "STATE_LABELS");
        readMember<abs_type>(file_ip, no_state_clusters, "NO_CLUSTERS");
        readVecSet<abs_type>(file_ip, state_clusters, no_state_clusters, "STATE_CLUSTERS");
        std::vector<std::string*> control_input_labels, dist_input_labels;
        std::string* str=new std::string;
        *str="wr";
        *str+=std::to_string(p);
        control_input_labels.push_back(str);
        str=new std::string;
        *str="nop";
        *str+=std::to_string(p);
        control_input_labels.push_back(str);
        str=new string;
        *str="wr";
        *str+=std::to_string(1-p);
        dist_input_labels.push_back(str);
        str=new string;
        *str="nop";
        *str+=std::to_string(1-p);
        dist_input_labels.push_back(str);
        std::string file_op="Outputs/pr_";
        file_op+=std::to_string(p);
        file_op+=".gv";
        std::string graph_name="Process_";
        graph_name+=std::to_string(p);
        N.components_[p]->createDOT(file_op, graph_name, state_labels, state_clusters, control_input_labels, dist_input_labels);
        /* save the contract */
        file_op="Outputs/guarantee_";
        file_op+=std::to_string(p);
        file_op+=".gv";
        graph_name="Guarantee_";
        graph_name+=std::to_string(p);
        N.guarantee_[p]->createDOT(file_op, graph_name, dist_input_labels);
    }

    TicToc timer;
    /* perform the negotiation */
    timer.tic();
    N.iterative_deepening_search();
    timer.toc();

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
