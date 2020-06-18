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
    std::string* c1= new std::string("Inputs/feeder.txt");
    std::string* c2= new std::string("Inputs/plant.txt");
    component_files.push_back(c1);
    component_files.push_back(c2);

    std::string* s1= new std::string("Inputs/safe_states_feeder.txt");
    std::string* s2= new std::string("Inputs/safe_states_plant.txt");
    safe_states_files.push_back(s1);
    safe_states_files.push_back(s2);

    std::string* t1= new std::string("Inputs/target_states_feeder.txt");
    std::string* t2= new std::string("Inputs/target_states_plant.txt");
    target_states_files.push_back(t1);
    target_states_files.push_back(t2);

    negotiation::Negotiate N(component_files, safe_states_files, target_states_files, k_max);

//    /* save the transition systems in DOT language */
//    for (int p=0; p<2; p++) {
//        std::vector<std::string*> state_labels;
//        abs_type no_state_clusters;
//        std::vector<std::unordered_set<abs_type>*> state_clusters;
//        std::string file_ip="Inputs/pr_";
//        file_ip+=std::to_string(p);
//        file_ip+=".txt";
//        readVec<std::string>(file_ip, state_labels, (size_t)N.components_[p]->no_states, "STATE_LABELS");
//        readMember<abs_type>(file_ip, no_state_clusters, "NO_CLUSTERS");
//        readVecSet<abs_type>(file_ip, state_clusters, no_state_clusters, "STATE_CLUSTERS");
//        std::vector<std::string*> control_input_labels, dist_input_labels;
//        std::string* str=new std::string;
//        *str="wr";
//        *str+=std::to_string(p);
//        control_input_labels.push_back(str);
//        str=new std::string;
//        *str="nop";
//        *str+=std::to_string(p);
//        control_input_labels.push_back(str);
//        str=new string;
//        *str="wr";
//        *str+=std::to_string(1-p);
//        dist_input_labels.push_back(str);
//        str=new string;
//        *str="nop";
//        *str+=std::to_string(1-p);
//        dist_input_labels.push_back(str);
//        std::string file_op="Outputs/pr_";
//        file_op+=std::to_string(p);
//        file_op+=".gv";
//        std::string graph_name="Process_";
//        graph_name+=std::to_string(p);
//        N.components_[p]->createDOT(file_op, graph_name, state_labels, state_clusters, control_input_labels, dist_input_labels);
//        /* save the contract */
//        file_op="Outputs/guarantee_";
//        file_op+=std::to_string(p);
//        file_op+=".gv";
//        graph_name="Guarantee_";
//        graph_name+=std::to_string(p);
//        N.guarantee_[p]->createDOT(file_op, graph_name, dist_input_labels);
//    }

    TicToc timer;
    /* perform the negotiation */
    timer.tic();
    int k=N.iterative_deepening_search();
    double elapsed_time=timer.toc();

    checkMakeDir("Outputs");
    N.guarantee_[0]->writeToFile("Outputs/guarantee_feeder.txt");
    N.guarantee_[1]->writeToFile("Outputs/guarantee_plant.txt");

//    /* save the contracts in DOT language */
//    for (int p=0; p<2; p++) {
//        std::vector<std::string*> output_labels;
//        std::string* str=new std::string;
//        *str="wr";
//        *str+=std::to_string(p);
//        output_labels.push_back(str);
//        str=new std::string;
//        *str="nop";
//        *str+=std::to_string(p);
//        output_labels.push_back(str);
//        /* save the contract */
//        std::string file_op="Outputs/guarantee_";
//        file_op+=std::to_string(p);
//        file_op+=".gv";
//        std::string graph_name="Guarantee_";
//        graph_name+=std::to_string(p);
//        N.guarantee_[p]->createDOT(file_op, graph_name, output_labels);
//    }
    
    /* get the current directory name */
    char buff[FILENAME_MAX]; //create string buffer to hold path
    GetCurrentDir( buff, FILENAME_MAX );
    std::string current_working_dir(buff);
    /* get the parameters of this example (the substring following "factory_") */
    char substr[10]="factory_";
    char params[20];
    for (int i=0; buff[i]!='\0'; i++) {
        int j=0;
        if (buff[i]==substr[j]) {
            int temp=i+1;
            while(buff[i]==substr[j])
            {
                i++;
                j++;
            }
            if(substr[j]=='\0')
            {
                for (int k=0; buff[i]!='\0'; i++, k++) {
                     params[k]=buff[i];
                }
            }
            else
            {
                i=temp;
            }
        }
    }
    /* save results to the log file */
    char file_name1[40]="../results_iterative_search.log";
    FILE* logfile1=fopen(file_name1, "a");
    if (logfile1!=NULL) {
        if (k==-1) {
            fprintf(logfile1, "%s \t|X0|=%i \t|X1|=%i \tG0:%i states \tG1:%i states \tk=%i \ttime=%f sec\tCONTRACT DOESN'T EXIST\n", params, N.components_[0]->no_states, N.components_[1]->no_states, N.guarantee_[0]->no_states_, N.guarantee_[1]->no_states_, -1, elapsed_time);
        } else if (k > k_max) {
            fprintf(logfile1, "%s \t|X0|=%i \t|X1|=%i \tG0:%i states \tG1:%i states \tk=%i \ttime=%f sec\tINCONCLUSIVE NEGOTIATION\n", params, N.components_[0]->no_states, N.components_[1]->no_states, N.guarantee_[0]->no_states_, N.guarantee_[1]->no_states_, k, elapsed_time);
        } else {
            fprintf(logfile1, "%s \t|X0|=%i \t|X1|=%i \tG0:%i states \tG1:%i states \tk=%i \ttime=%f sec\tSUCCESSFUL NEGOTIATION\n", params, N.components_[0]->no_states, N.components_[1]->no_states, N.guarantee_[0]->no_states_, N.guarantee_[1]->no_states_, k, elapsed_time);
            /* perform negotiation with fixed k-minimization */
            char file_name2[40]="../results_fixed_depth_search.log";
            FILE* logfile2=fopen(file_name2, "a");
            TicToc timer_fixed_minimization;
            for (int l=0; l<=k; l++) {
                timer_fixed_minimization.tic();
                int out=N.fixed_depth_search(l, 0);
                double elapsed_time_fixed_minimization=timer_fixed_minimization.toc();
                /* save results to the log file */
                if (logfile2!=NULL) {
                    fprintf(logfile2, "%s \t|X0|=%i \t|X1|=%i \tG0:%i states \tG1:%i states \tk=%i \ttime=%f sec\n", params, N.components_[0]->no_states, N.components_[1]->no_states, N.guarantee_[0]->no_states_, N.guarantee_[1]->no_states_, l, elapsed_time_fixed_minimization);
                }
            }
        }
        
    }
    /* perform negotiation without the k-minimization */
    TicToc timer_wo_minimization;
    timer_wo_minimization.tic();
    bool flag=false;
    N.reset();
    N.recursive_negotiation(INT_MAX, 0, 0, flag);
    double elapsed_time_wo_minimization=timer_wo_minimization.toc();
    /* save results to the log file */
    char file_name3[40]="../results_plain_negotiation.log";
    FILE* logfile3=fopen(file_name3, "a");
    if (logfile3!=NULL) {
        if (k==-1) {
            fprintf(logfile3, "%s \t|X0|=%i \t|X1|=%i \tG0:%i states \tG1:%i states \tk=%i \ttime=%f sec\tCONTRACT DOESN'T EXIST\n", params, N.components_[0]->no_states, N.components_[1]->no_states, N.guarantee_[0]->no_states_, N.guarantee_[1]->no_states_, -1, elapsed_time_wo_minimization);
        } else if (k > k_max) {
            fprintf(logfile3, "%s \t|X0|=%i \t|X1|=%i \tG0:%i states \tG1:%i states \tk=%i \ttime=%f sec\tINCONCLUSIVE NEGOTIATION\n", params, N.components_[0]->no_states, N.components_[1]->no_states, N.guarantee_[0]->no_states_, N.guarantee_[1]->no_states_, k, elapsed_time_wo_minimization);
        } else {
            fprintf(logfile3, "%s \t|X0|=%i \t|X1|=%i \tG0:%i states \tG1:%i states \tk=%i \ttime=%f sec\tSUCCESSFUL NEGOTIATION\n", params, N.components_[0]->no_states, N.components_[1]->no_states, N.guarantee_[0]->no_states_, N.guarantee_[1]->no_states_, k, elapsed_time_wo_minimization);
        }
    }

    return 1;
}
