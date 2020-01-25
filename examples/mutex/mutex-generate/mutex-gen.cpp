/*
 * mutex-gen.cpp
 *
 *  Created on: 25.01.2020
 *      author: kaushik
 */

 /*
  * A program to automatically generate the mutual exclusion example in a parameterized way.
  */

 #include <stdlib.h>

 // #include "FileHandler.hpp"
 #include "Component.hpp" /* for the definition of the data type abs_type */

 /* The parameters */
 #define pr1_deadline_ 15
 #define pr2_deadline_ 20
 #define pr1_max_period_ 3
 #define pr2_max_period_ 2
 #define pr1_data_size_ 8
 #define pr2_data_size_ 5

using namespace std;
using namespace negotiation;

/*********************************************************/
/* main computation */
/*********************************************************/
int main() {
    /* make a folder for the given parameters */
    std::string Str_folder = "";
    Str_folder += "../mutex_dl_";
    Str_folder += std::to_string(pr1_deadline_);
    Str_folder += "_";
    Str_folder += std::to_string(pr2_deadline_);
    Str_folder += "_mp_";
    Str_folder += std::to_string(pr1_max_period_);
    Str_folder += "_";
    Str_folder += str::to_string(pr2_max_period_);
    Str_folder += "_ds_";
    Str_folder += std::to_string(pr1_data_size_);
    Str_folder += "_";
    Str_folder += std::to_string(pr2_data_size_);
    char folder_name[30];
    size_t Length = Str_folder.copy(folder_name, Str_folder.length() + 1);
    folder_name[Length] = '\0';
    checkMakeDir(folder_name);

    std::string Str_input_folder = Str_folder;
    Str_input_folder += "/Inputs";
    char input_folder_name[20];
    Length = Str_input_folder.copy(input_folder_name, Str_input_folder.length() + 1);
    input_folder_name[Length] = '\0';
    checkMakeDir(input_folder_name);

    /* put the parameters in an array */
    int* dl[2]={pr1_deadline_, pr2_deadline_};
    int* mp[2]={pr1_max_period_,pr2_max_period_};
    int* ds[2]={pr1_data_size_,pr2_data_size_};

    /* create the processes */
    for (int pid=0; pid<2; pid++) {
        // /* the global countdown timer for the deadline check */
        // int glob_time;
        // /* the local countdown timer for the periodicity check
        //  * started for the first time when a successful write happens for the first time */
        // int loc_time;

        /* There are states related to the writing process each having four components:
         *   1: the state of one write operation ("idle" (0), "write" (1), "conflict check" (2), "conflict" (3), or "success" (4))
         *   2: the number of packets remaining to be written (0-data size)
         *   3: the time remaining in the global timer (0-deadline)
         *   4: the time remaining until the next packet (0-max period).
         * In addition, there are three sink states:
         *   1: overall time-out (bad) -> state id 0
         *   2: period time-out (bad) -> state id 1
         *   3: task completed (good) -> state id 2 */
        abs_type no_states = 5*(ds[pid]-1)*(dl[pid]-1)*(mp[pid]-1)+3;
        /* the id of the writing related states are determined using the following lambda expression */
        auto state_id = [&](abs_type i, abs_type j, abs_type k, abs_type l) -> abs_type {
            if (k==0) {
                /* overall time-out */
                return 0;
            } else if (l==0) {
                /* period time-out */
                return 1;
            } else if (j==0) {
                /* task completed */
                return 2;
            } else {
                return (i*ds[pid]*dl[pid]*mp[pid] + j*dl[pid]*mp[pid] + k*mp[pid] + l + 3);
            }
        };
        /* the initial state is the state "idle" with all the clock being set to maximum limit */
        std::unordered_set<abs_type> init;
        init.insert(state_id(0,ds[pid]-1,dl[pid]-1,mp[pid]-1));
        /* control inputs:
         *  0: write
         *  1: wait
         */
        abs_type no_control_input = 2;
        /* disturbance inputs:
         *  0: other process writing
         *  1: other process idle
         */
        abs_type no_dist_input = 2;
        /* outputs:
         *  0: writing
         *  1: idle
         */
        abs_type no_output = 2;
        /* state-to-output map */
        std::vector<abs_type> state_to_output;
        /* the sink states are in idle all the time */
        state_to_output.push_back(1);
        state_to_output.push_back(1);
        state_to_output.push_back(1);
        /* the post array */
        std::vector<abs_type>** post = new std::vector<abs_type>*[no_states*no_control_input*no_dist_input];
        /* iterate over all the states to compute the post */
        int ind=0;
        for (int i=0; i<=4; i++) {
            for (int j=ds[pid]-1; j>=0; j--) {
                for (int k=dl[pid]-1; k>=0; k--) {
                    for (int l=mp[pid]-1; l>=0; l--) {
                        /* the output of a state is determined by the first component */
                        switch (i) {
                            case 0: state_to_output.push_back(1);
                            case 1: state_to_output.push_back(0);
                            case 2: state_to_output.push_back(1);
                            case 3: state_to_output.push_back(1);
                            case 4: state_to_output.push_back(1);
                        }
                        for (int u=0; u<no_control_input; u++) {
                            for (int w=0; w<no_dist_input; w++) {
                                std::vector<abs_type>* v = new std::vector<abs_type>;
                                post[ind]=v;
                                ind++;
                            }
                        }
                    }
                }
            }
        }
        /* the address of the post vector in the "post" array is determined using the following lambda expression */
        auto post_addr = [&](abs_type x, abs_type u, abs_type w) -> abs_type {
            return (x*no_control_inputs*no_dist_inputs + u*no_dist_inputs + w);
        };
        /* iterate over all the states to compute the post */
        for (int j=ds[pid]-1; j>=1; j--) {
            for (int k=dl[pid]-1; k>=1; k--) {
                for (int l=mp[pid]-1; l>=1; l--) {
                    abs_type l_updated;
                    /* don't decrement the period timer until the first time a write has occurred */
                    if (j==ds[pid]-1) {
                        l_updated = l;
                    } else {
                        l_updated = l-1;
                    }
                    /* state "idle" */
                    abs_type x = state_id(0,j,k,l);
                    post[post_addr(x,0,0)]->push_back(state_id(1,j,k-1,l_updated));
                    post[post_addr(x,0,1)]->push_back(state_id(1,j,k-1,l_updated));
                    post[post_addr(x,1,0)]->push_back(state_id(0,j,k-1,l_updated));
                    post[post_addr(x,1,1)]->push_back(state_id(0,j,k-1,l_updated));
                    /* state "write" */
                    x = state_id(1,j,k,l);
                    post[post_addr(x,0,0)]->push_back(state_id(2,j,k-1,l_updated));
                    post[post_addr(x,0,1)]->push_back(state_id(2,j,k-1,l_updated));
                    /* state "conflict check" */
                    x = state_id(2,j,k,l);
                    post[post_addr(x,0,0)]->push_back(state_id(3,j,k-1,l_updated));
                    post[post_addr(x,0,1)]->push_back(state_id(4,j,k-1,l_updated));
                    /* state "conflict" */
                    x = state_id(3,j,k,l);
                    post[post_addr(x,0,0)]->push_back(state_id(1,j,k-1,l_updated));
                    post[post_addr(x,0,1)]->push_back(state_id(1,j,k-1,l_updated));
                    post[post_addr(x,1,0)]->push_back(state_id(0,j,k-1,l_updated));
                    post[post_addr(x,1,1)]->push_back(state_id(0,j,k-1,l_updated));
                    /* state "success" */
                    x = state_id(4,j,k,l);
                    post[post_addr(x,0,0)]->push_back(state_id(1,j-1,k-1,mp[pid]-1));
                    post[post_addr(x,0,1)]->push_back(state_id(1,j-1,k-1,mp[pid]-1));
                    post[post_addr(x,1,0)]->push_back(state_id(0,j-1,k-1,mp[pid]-1));
                    post[post_addr(x,1,1)]->push_back(state_id(0,j-1,k-1,mp[pid]-1));
                }
            }
        }
        /* write the description of a file */
        std::string Str_file = Str_input_folder;
        Str_file += "/pr"
        Str_file += std::to_string(pid);
        Str_file += ".txt";
        create(Str_file);
        writeMember<abs_type>(Str_file, "NO_STATES", no_states);
        writeMember<int>(Str_file, "NO_INITIAL_STATES", init.size());
        writeSet<abs_type>(Str_file, "INITIAL_STATE_LIST", init);
        writeMember<abs_type>(Str_file, "NO_CONTROL_INPUTS", no_control_inputs);
        writeMember<abs_type>(Str_file, "NO_DIST_INPUTS", no_dist_inputs);
        writeMember<abs_type>(Str_file, "NO_OUTPUTS", no_outputs);
        writeVec<abs_type>(Str_file, "STATE_TO_OUTPUT", state_to_output);
        writeArrVec<abs_type>(Str_file, "TRANSITION_POST", post, no_states*no_control_inputs*no_dist_inputs);

    }



    /* create the safe set for the feeder: all states except "shutdown" are safe */
    std::unordered_set<abs_type> safe_states_feeder;
    for (abs_type i=0; i<no_states_feeder-1; i++) {
        safe_states_feeder.insert(i);
    }
    Str_file = Str_input_folder;
    Str_file += "/safe_states_feeder.txt";
    create(Str_file);
    writeMember<abs_type>(Str_file, "NO_SAFE_STATES", safe_states_feeder.size());
    writeSet<abs_type>(Str_file, "SET_SAFE_STATES", safe_states_feeder);

    /* create the safe set for the plant: all states are safe */
    std::unordered_set<abs_type> safe_states_plant;
    for (abs_type i=0; i<no_states_plant; i++) {
        safe_states_plant.insert(i);
    }
    Str_file = Str_input_folder;
    Str_file += "/safe_states_plant.txt";
    create(Str_file);
    writeMember<abs_type>(Str_file, "NO_SAFE_STATES", safe_states_plant.size());
    writeSet<abs_type>(Str_file, "SET_SAFE_STATES", safe_states_plant);

    /* create the target states for the feeder: all states are in the target */
    std::unordered_set<abs_type> target_states_feeder;
    for (abs_type i=0; i<no_states_feeder; i++) {
        target_states_feeder.insert(i);
    }
    Str_file = Str_input_folder;
    Str_file += "/target_states_feeder.txt";
    create(Str_file);
    writeMember<abs_type>(Str_file, "NO_TARGET_STATES", target_states_feeder.size());
    writeSet<abs_type>(Str_file, "SET_TARGET_STATES", target_states_feeder);

    /* create the target states for the plant: the two states with the highest hibernating time are in the target */
    std::unordered_set<abs_type> target_states_plant;
    if (plant_hibernate_cycle_>1) {
        target_states_plant.insert(2*plant_process_cycles_+plant_hibernate_cycle_);
        target_states_plant.insert(no_states_plant-1);
    } else {
        target_states_plant.insert(0);
        target_states_plant.insert(1);
    }
    Str_file = Str_input_folder;
    Str_file += "/target_states_plant.txt";
    create(Str_file);
    writeMember<abs_type>(Str_file, "NO_TARGET_STATES", target_states_plant.size());
    writeSet<abs_type>(Str_file, "SET_TARGET_STATES", target_states_plant);

    /* copy other necessary files */
    std::string Str_copy = "cp files/Makefile ";
    Str_folder += "/";
    Str_copy += Str_folder;
    char char_copy[42];
    Length = Str_copy.copy(char_copy, Str_copy.length() + 1);
    char_copy[Length] = '\0';
    system(char_copy);
    Str_copy = "cp files/factory.cpp ";
    Str_copy += Str_folder;
    Length = Str_copy.copy(char_copy, Str_copy.length() + 1);
    char_copy[Length] = '\0';
    system(char_copy);

    delete[] post_feeder;
    delete[] post_plant;

    return 1;
}
