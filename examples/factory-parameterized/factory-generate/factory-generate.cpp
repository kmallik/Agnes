/*
 * factory-generate.cpp
 *
 *  Created on: 21.01.2020
 *      author: kaushik
 */

 /*
  * A program to automatically generate the feeder-plant example in a parameterized way.
  */

 #include <stdlib.h>

 // #include "FileHandler.hpp"
 #include "Component.hpp" /* for the definition of the data type abs_type */

 /* The parameters */
 #define plant_process_cycles_ 1
 #define plant_hibernate_cycle_ 1
 #define feeder_max_wait_cycles_ 3

using namespace std;
using namespace negotiation;

/*********************************************************/
/* main computation */
/*********************************************************/
int main() {
    /* sanity check */
    if (plant_process_cycles_<1) {
        throw std::invalid_argument("The plant process cycle should be at least 1.\n");
    }
    /* make a folder for the given parameters */
    std::string Str_folder = "";
    Str_folder += "../factory_pp";
    Str_folder += std::to_string(plant_process_cycles_);
    Str_folder += "_ph";
    Str_folder += std::to_string(plant_hibernate_cycle_);
    Str_folder += "_fw";
    Str_folder += std::to_string(feeder_max_wait_cycles_);
    char folder_name[20];
    size_t Length = Str_folder.copy(folder_name, Str_folder.length() + 1);
    folder_name[Length] = '\0';
    checkMakeDir(folder_name);

    std::string Str_input_folder = Str_folder;
    Str_input_folder += "/Inputs";
    char input_folder_name[20];
    Length = Str_input_folder.copy(input_folder_name, Str_input_folder.length() + 1);
    input_folder_name[Length] = '\0';
    checkMakeDir(input_folder_name);
    /* generate the feeder model */
    /* the number of states constitutes of 4 fixed states:
     * 0: "empty"
     * 1: "part full"
     * 2: "full"
     * no_states-1: "shutdown"
     *
     * and the pairs of variable states (counting the number of times the feeder is idle ):
     * odd numbers between 2 and no_states-2: "idle+not full"
     * even numbers between 3 and no_states-1: "idle+full"
     */
    abs_type no_states_feeder=4+2*(feeder_max_wait_cycles_-1);
    /* the initial state is the state "empty" */
    std::unordered_set<abs_type> init_feeder;
    init_feeder.insert(0);
    /* the control inputs are:
     * 0: "push"
     * 1: "wait"
     */
    abs_type no_control_inputs_feeder=2;
    /* the disturbance inputs are:
     * 0: "plant idle"
     * 1: "plant busy and not consuming items"
     * 2: "plant just consumed one item"
     */
    abs_type no_dist_inputs_feeder=3;
    /* the outputs are:
     * 0: "feeder idle"
     * 1: "feeder busy" */
    abs_type no_outputs_feeder=2;
    /* state to output map:
     * "part full" -> "feeder busy"
     * "full" -> "feeder busy"
     * rest -> "feeder idle" */
    std::vector<abs_type> state_to_output_feeder;
    for (abs_type i=0; i<no_states_feeder; i++) {
        if (i==1 || i==2) {
            state_to_output_feeder.push_back(1);
        } else {
            state_to_output_feeder.push_back(0);
        }
    }
    /* the transition array */
    std::vector<abs_type>** post_feeder=new std::vector<abs_type>*[no_states_feeder*no_control_inputs_feeder*no_dist_inputs_feeder];
    for (abs_type i=0; i<no_states_feeder*no_control_inputs_feeder*no_dist_inputs_feeder; i++) {
        std::vector<abs_type>* v= new std::vector<abs_type>;
        post_feeder[i]=v;
    }
    /* the address of the post vector in the "post_feeder" array is determined using the following lambda expression */
        auto post_addr_feeder = [&](abs_type i, abs_type j, abs_type k) -> abs_type {
            return (i*no_control_inputs_feeder*no_dist_inputs_feeder + j*no_dist_inputs_feeder + k);
        };
    /* the transitions from states 0 to 2 and state no_states are fixed */
    post_feeder[post_addr_feeder(0,0,0)]->push_back(1);
    post_feeder[post_addr_feeder(0,0,1)]->push_back(1);
    post_feeder[post_addr_feeder(0,0,2)]->push_back(1);
    post_feeder[post_addr_feeder(0,1,0)]->push_back(3);
    post_feeder[post_addr_feeder(0,1,1)]->push_back(3);
    post_feeder[post_addr_feeder(0,1,2)]->push_back(3);

    post_feeder[post_addr_feeder(1,0,0)]->push_back(1);
    post_feeder[post_addr_feeder(1,0,0)]->push_back(2);
    post_feeder[post_addr_feeder(1,0,1)]->push_back(1);
    post_feeder[post_addr_feeder(1,0,1)]->push_back(2);
    post_feeder[post_addr_feeder(1,0,2)]->push_back(1);
    post_feeder[post_addr_feeder(1,1,0)]->push_back(3);
    post_feeder[post_addr_feeder(1,1,1)]->push_back(3);
    post_feeder[post_addr_feeder(1,1,2)]->push_back(3);

    if (feeder_max_wait_cycles_>1) {
        post_feeder[post_addr_feeder(2,1,0)]->push_back(4);
        post_feeder[post_addr_feeder(2,1,1)]->push_back(4);
    } else {
        post_feeder[post_addr_feeder(2,1,0)]->push_back(3);
        post_feeder[post_addr_feeder(2,1,1)]->push_back(3);
    }
    post_feeder[post_addr_feeder(2,1,2)]->push_back(3);

    /* transitions from rest of the states */
    for (abs_type i=3; i<no_states_feeder-2; i++) {
        if (i%2!=0) {
            post_feeder[post_addr_feeder(i,0,0)]->push_back(1);
            post_feeder[post_addr_feeder(i,0,0)]->push_back(2);
            post_feeder[post_addr_feeder(i,0,1)]->push_back(1);
            post_feeder[post_addr_feeder(i,0,1)]->push_back(2);
            post_feeder[post_addr_feeder(i,0,2)]->push_back(1);
            post_feeder[post_addr_feeder(i,1,0)]->push_back(i+2);
            post_feeder[post_addr_feeder(i,1,1)]->push_back(i+2);
            post_feeder[post_addr_feeder(i,1,2)]->push_back(i+2);
        } else {
            post_feeder[post_addr_feeder(i,1,0)]->push_back(i+2);
            post_feeder[post_addr_feeder(i,1,1)]->push_back(i+2);
            post_feeder[post_addr_feeder(i,1,2)]->push_back(i-1);
        }
    }

    if (feeder_max_wait_cycles_>1) {
        post_feeder[post_addr_feeder(no_states_feeder-2,1,0)]->push_back(no_states_feeder-1);
        post_feeder[post_addr_feeder(no_states_feeder-2,1,1)]->push_back(no_states_feeder-1);
        post_feeder[post_addr_feeder(no_states_feeder-2,1,2)]->push_back(no_states_feeder-1);
    }

    /* self loop in the shutdown state */
    post_feeder[post_addr_feeder(no_states_feeder-1,0,0)]->push_back(no_states_feeder-1);
    post_feeder[post_addr_feeder(no_states_feeder-1,0,1)]->push_back(no_states_feeder-1);
    post_feeder[post_addr_feeder(no_states_feeder-1,0,2)]->push_back(no_states_feeder-1);
    post_feeder[post_addr_feeder(no_states_feeder-1,1,0)]->push_back(no_states_feeder-1);
    post_feeder[post_addr_feeder(no_states_feeder-1,1,1)]->push_back(no_states_feeder-1);
    post_feeder[post_addr_feeder(no_states_feeder-1,1,2)]->push_back(no_states_feeder-1);

    /* write the description of the feeder model */
    std::string Str_file = Str_input_folder;
    Str_file += "/feeder.txt";
    create(Str_file);
    writeMember<abs_type>(Str_file, "NO_STATES", no_states_feeder);
    writeMember<int>(Str_file, "NO_INITIAL_STATES", init_feeder.size());
    writeSet<abs_type>(Str_file, "INITIAL_STATE_LIST", init_feeder);
    writeMember<abs_type>(Str_file, "NO_CONTROL_INPUTS", no_control_inputs_feeder);
    writeMember<abs_type>(Str_file, "NO_DIST_INPUTS", no_dist_inputs_feeder);
    writeMember<abs_type>(Str_file, "NO_OUTPUTS", no_outputs_feeder);
    writeVec<abs_type>(Str_file, "STATE_TO_OUTPUT", state_to_output_feeder);
    writeArrVec<abs_type>(Str_file, "TRANSITION_POST", post_feeder, no_states_feeder*no_control_inputs_feeder*no_dist_inputs_feeder);

    /* *********************************** */
    /* generate the plant model */
    /* *********************************** */
    /* the difference between plant hibernate cycle=1 and =0 is that the set of target states is empty in the later case */
    abs_type plant_effective_hibernate_cycle_;
    if (plant_hibernate_cycle_<0) {
        throw std::invalid_argument("Invalid argument for the plant hibernate cycle.\n");
    } else if (plant_hibernate_cycle_==0) {
        plant_effective_hibernate_cycle_=1;
    } else {
        plant_effective_hibernate_cycle_=plant_hibernate_cycle_;
    }
    /* the number of states constitutes of 4 fixed states:
     * 0: "idle, empty"
     * 1: "idle, non empty"
     * 2: "beginning of a process cycle, empty"
     * 3: "beginning of a process cycle, non-empty"
     *
     * pairs of variable states (counting the cycles the plant has been processing the current job):
     * even numbers between 1 and 2*plant_process_cycles_+1: "busy, empty"
     * odd numbers between 2 and 2*plant_process_cycles_+2: "busy, non empty"
     *
     * the variable states (counting the number of cycles the plant has spent hibernating, if plant_effective_hibernate_cycle_>=2):
     * (2*plant_process_cycles_+2) to
        ((2*plant_process_cycles_+2) + (plant_effective_hibernate_cycle_-2): "idle, empty"
     * ((2*plant_process_cycles_+2) + (plant_effective_hibernate_cycle_-2) + 1) to
        ((2*plant_process_cycles_+2) + (plant_effective_hibernate_cycle_-2) + 1 + (plant_effective_hibernate_cycle_-2)): "idle, non empty"
     */
    abs_type no_states_plant;
    no_states_plant= 4 + 2*(plant_process_cycles_-1) + 2*(plant_effective_hibernate_cycle_-1);

    /* the initial state is the state "idle, empty" */
    std::unordered_set<abs_type> init_plant;
    init_plant.insert(0);
    /* the control inputs are:
     * 0: "process"
     * 1: "wait" */
    abs_type no_control_inputs_plant=2;
    /* the disturbance inputs are:
     * 0: "feeder idle"
     * 1: "feeder busy" */
    abs_type no_dist_inputs_plant=2;
    /* the outputs are:
     * 0: "plant idle"
     * 1: "plant busy and not consuming item"
     * 2: "plant just consumed one item" */
    abs_type no_outputs_plant=3;
    /* state to output map:
     * "idle, empty" -> "plant idle"
     * "idle, non empty" -> "plant idle"
     * "beginning of process cycle, empty" -> "plant just consumed one item"
     * "beginning of process cycle, non empty" -> "plant just consumed one item"
     * the rest -> "plant busy and non consuming item" */
    std::vector<abs_type> state_to_output_plant;
    for (abs_type i=0; i<no_states_plant; i++) {
        if (i==0 || i==1 || i>=2*plant_process_cycles_+2) {
            state_to_output_plant.push_back(0);
        } else if (i==2 || i==3) {
            state_to_output_plant.push_back(2);
        } else {
            state_to_output_plant.push_back(1);
        }
    }
    /* the transition array */
    std::vector<abs_type>** post_plant=new std::vector<abs_type>*[no_states_plant*no_control_inputs_plant*no_dist_inputs_plant];
    for (abs_type i=0; i<no_states_plant*no_control_inputs_plant*no_dist_inputs_plant; i++) {
        std::vector<abs_type>* v= new std::vector<abs_type>;
        post_plant[i]=v;
    }
    /* the address of the post vector in the "post_plant" array is determined using the following lambda expression */
        auto post_addr_plant = [&](abs_type i, abs_type j, abs_type k) -> abs_type {
            return (i*no_control_inputs_plant*no_dist_inputs_plant + j*no_dist_inputs_plant + k);
        };

    /* outgoing transitions from idle, empty state */
    if (plant_effective_hibernate_cycle_>1) {
        post_plant[post_addr_plant(0,1,0)]->push_back(2*plant_process_cycles_+2);
    } else {
        post_plant[post_addr_plant(0,1,0)]->push_back(0);
    }
    if (plant_effective_hibernate_cycle_>2) {
        post_plant[post_addr_plant(0,1,1)]->push_back(no_states_plant - (plant_effective_hibernate_cycle_-1));
    } else {
        post_plant[post_addr_plant(0,1,1)]->push_back(1);
    }

    /* outgoing transitions from idle, non empty states */
    post_plant[post_addr_plant(1,0,0)]->push_back(3);
    post_plant[post_addr_plant(1,0,0)]->push_back(2);
    post_plant[post_addr_plant(1,0,1)]->push_back(3);
    if (plant_effective_hibernate_cycle_>2) {
        post_plant[post_addr_plant(1,1,0)]->push_back(no_states_plant-plant_effective_hibernate_cycle_+2);
        post_plant[post_addr_plant(1,1,1)]->push_back(no_states_plant-plant_effective_hibernate_cycle_+2);
    } else {
        post_plant[post_addr_plant(1,1,0)]->push_back(1);
        post_plant[post_addr_plant(1,1,1)]->push_back(1);
    }

    /* busy states */
    for (abs_type i=2; i<=2*plant_process_cycles_+1; i++) {
        if (i<2*plant_process_cycles_ && plant_process_cycles_>1) {
            /* the intermediate process cycle states */
            if (i%2==0) {
                /* busy, empty states */
                post_plant[post_addr_plant(i,0,0)]->push_back(i+2);
                post_plant[post_addr_plant(i,0,1)]->push_back(i+3);
            } else {
                /* busy, non-empty states */
                post_plant[post_addr_plant(i,0,0)]->push_back(i+2);
                post_plant[post_addr_plant(i,0,1)]->push_back(i+2);
            }
        } else {
            /* the end of process cycle states */
            if (i%2==0) {
                /* busy, empty states */
                post_plant[post_addr_plant(i,1,0)]->push_back(0);
                if (plant_effective_hibernate_cycle_==1) {
                    post_plant[post_addr_plant(i,1,1)]->push_back(1);
                } else {
                    post_plant[post_addr_plant(i,1,1)]->push_back(no_states_plant - (plant_effective_hibernate_cycle_-1));
                }
            } else {
                /* busy, non-empty states */
                post_plant[post_addr_plant(i,0,0)]->push_back(2);
                post_plant[post_addr_plant(i,0,0)]->push_back(3);
                post_plant[post_addr_plant(i,0,1)]->push_back(3);
                if (plant_effective_hibernate_cycle_==1) {
                    post_plant[post_addr_plant(i,1,0)]->push_back(1);
                    post_plant[post_addr_plant(i,1,1)]->push_back(1);
                } else {
                    post_plant[post_addr_plant(i,1,0)]->push_back(no_states_plant - (plant_effective_hibernate_cycle_-1));
                    post_plant[post_addr_plant(i,1,1)]->push_back(no_states_plant - (plant_effective_hibernate_cycle_-1));
                }
            }
        }
    }

    /* hibernating (idle, empty) states */
    for (abs_type i=2*plant_process_cycles_+2, counter=plant_effective_hibernate_cycle_-1; counter>0; i++, counter--) {
        if (counter>1) {
            post_plant[post_addr_plant(i,1,0)]->push_back(i+1);
            post_plant[post_addr_plant(i,1,1)]->push_back(no_states_plant-counter+1);
        } else {
            post_plant[post_addr_plant(i,1,0)]->push_back(i);
            post_plant[post_addr_plant(i,1,1)]->push_back(1);
        }
    }

    /* hibernating (idle, non-empty) states */
    for (abs_type i=no_states_plant - (plant_effective_hibernate_cycle_-1); i<no_states_plant; i++) {
        post_plant[post_addr_plant(i,0,0)]->push_back(2);
        post_plant[post_addr_plant(i,0,0)]->push_back(3);
        post_plant[post_addr_plant(i,0,1)]->push_back(3);
        if (i<no_states_plant-1) {
            post_plant[post_addr_plant(i,1,0)]->push_back(i+1);
            post_plant[post_addr_plant(i,1,1)]->push_back(i+1);
        } else {
            post_plant[post_addr_plant(i,1,0)]->push_back(1);
            post_plant[post_addr_plant(i,1,1)]->push_back(1);
        }
    }

    /* write the description of the plant model */
    Str_file = Str_input_folder;
    Str_file += "/plant.txt";
    create(Str_file);
    writeMember<abs_type>(Str_file, "NO_STATES", no_states_plant);
    writeMember<int>(Str_file, "NO_INITIAL_STATES", init_plant.size());
    writeSet<abs_type>(Str_file, "INITIAL_STATE_LIST", init_plant);
    writeMember<abs_type>(Str_file, "NO_CONTROL_INPUTS", no_control_inputs_plant);
    writeMember<abs_type>(Str_file, "NO_DIST_INPUTS", no_dist_inputs_plant);
    writeMember<abs_type>(Str_file, "NO_OUTPUTS", no_outputs_plant);
    writeVec<abs_type>(Str_file, "STATE_TO_OUTPUT", state_to_output_plant);
    writeArrVec<abs_type>(Str_file, "TRANSITION_POST", post_plant, no_states_plant*no_control_inputs_plant*no_dist_inputs_plant);

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
        target_states_plant.insert(no_states_plant-plant_effective_hibernate_cycle_);
        target_states_plant.insert(1);
    } else if (plant_hibernate_cycle_==1) {
        target_states_plant.insert(0);
        target_states_plant.insert(1);
    } else {
        /* all states are in the target */
        for (abs_type i=0; i<no_states_plant; i++) {
            target_states_plant.insert(i);
        }
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
