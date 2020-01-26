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
 #define pr2_deadline_ 15
 #define pr1_max_period_ 6
 #define pr2_max_period_ 6
 #define pr1_data_size_ 1
 #define pr2_data_size_ 1

using namespace std;
using namespace negotiation;

/* forward declaration of anonymous functions */
abs_type state_id(abs_type i, abs_type j, abs_type k, abs_type l);
abs_type post_addr(abs_type x, abs_type u, abs_type w);

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
    Str_folder += std::to_string(pr2_max_period_);
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
    char input_folder_name[35];
    Length = Str_input_folder.copy(input_folder_name, Str_input_folder.length() + 1);
    input_folder_name[Length] = '\0';
    checkMakeDir(input_folder_name);

    /* put the parameters in an array */
    int dl[2]={pr1_deadline_, pr2_deadline_};
    int mp[2]={pr1_max_period_,pr2_max_period_};
    int ds[2]={pr1_data_size_,pr2_data_size_};

    for (int pid=0; pid<2; pid++) {
        /* ******** create the processes ******** */
        /* There are states related to the writing process each having four components:
         *   1: the state of one write operation ("idle" (0), "write" (1), "conflict check" (2), "conflict" (3), or "success" (4))
         *   2: the number of packets remaining to be written (1 to data size)
         *   3: the time remaining in the global timer (deadline to 1)
         *   4: the time remaining until the next packet (max period to 1).
         * In addition, there are three sink states:
         *   1: overall time-out (bad) -> state id 0
         *   2: period time-out (bad) -> state id 1
         *   3: task completed (good) -> state id 2 */
        abs_type no_states = 5*ds[pid]*dl[pid]*mp[pid] + 3;
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
                return (i*ds[pid]*dl[pid]*mp[pid] + (j-1)*dl[pid]*mp[pid] + (k-1)*mp[pid] + (l-1) + 3);
            }
        };
        /* the initial state is the state "idle" with all the clock being set to maximum limit */
        std::unordered_set<abs_type> init;
        init.insert(state_id(0,ds[pid],dl[pid],mp[pid]));
        /* control inputs:
         *  0: write
         *  1: wait
         */
        abs_type no_control_inputs = 2;
        /* disturbance inputs:
         *  0: other process writing
         *  1: other process idle
         */
        abs_type no_dist_inputs = 2;
        /* outputs:
         *  0: writing
         *  1: idle
         */
        abs_type no_outputs = 2;
        /* state-to-output map */
        std::vector<abs_type> state_to_output;
        /* initialize state_to_output vector */
        for (int i=0; i<no_states; i++) {
            state_to_output.push_back(0);
        }
        /* the three sink states are always idle */
        state_to_output[0]=1;
        state_to_output[1]=1;
        state_to_output[2]=1;
        for (int i=0; i<=4; i++) {
            for (int j=1; j<=ds[pid]; j++) {
                for (int k=1; k<=dl[pid]; k++) {
                    for (int l=1; l<=mp[pid]; l++) {
                        switch (i) {
                            case 0:
                                state_to_output[state_id(i,j,k,l)]=1;
                                break;
                            case 1:
                                state_to_output[state_id(i,j,k,l)]=0;
                                break;
                            case 2:
                                state_to_output[state_id(i,j,k,l)]=1;
                                break;
                            case 3:
                                state_to_output[state_id(i,j,k,l)]=1;
                                break;
                            case 4:
                                state_to_output[state_id(i,j,k,l)]=1;
                                break;
                        }
                    }
                }
            }
        }
        /* the post array */
        std::vector<abs_type>** post = new std::vector<abs_type>*[no_states*no_control_inputs*no_dist_inputs];
        /* initialize post */
        /* the sink states */
        for (int i=0; i<3*no_control_inputs*no_dist_inputs; i++) {
            std::vector<abs_type>* v = new std::vector<abs_type>;
            post[i]=v;
        }
        /* the other states */
        int ind=3*no_control_inputs*no_dist_inputs;
        for (int i=0; i<=4; i++) {
            for (int j=1; j<=ds[pid]; j++) {
                for (int k=1; k<=dl[pid]; k++) {
                    for (int l=1; l<=mp[pid]; l++) {
                        for (int u=0; u<no_control_inputs; u++) {
                            for (int w=0; w<no_dist_inputs; w++) {
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
        /* add self loops to the sink states */
        for (int i=0; i<3; i++) {
            for (int u=0; u<no_control_inputs; u++) {
                for (int w=0; w<no_dist_inputs; w++) {
                    post[post_addr(i,u,w)]->push_back(i);
                }
            }
        }
        /* iterate over all the states to compute the post */
        for (int j=ds[pid]; j>=1; j--) {
            for (int k=dl[pid]; k>=1; k--) {
                for (int l=mp[pid]; l>=1; l--) {
                    abs_type l_updated;
                    /* don't decrement the period timer until the first time a write has occurred */
                    if (j==ds[pid]) {
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
        Str_file += "/pr_";
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

        /* ****** create the specifications ******** */
        /* create the safe set: all the states except the time-out states (index 0 and 1) are safe */
        std::unordered_set<abs_type> safe_states;
        for (abs_type i=2; i<no_states; i++) {
            safe_states.insert(i);
        }
        Str_file = Str_input_folder;
        Str_file += "/safe_states_";
        Str_file += std::to_string(pid);
        Str_file += ".txt";
        create(Str_file);
        writeMember<abs_type>(Str_file, "NO_SAFE_STATES", safe_states.size());
        writeSet<abs_type>(Str_file, "SET_SAFE_STATES", safe_states);
        /* create the target state set for liveness spec: the "task completed" state (index 2) */
        std::unordered_set<abs_type> target_states;
        target_states.insert(2);
        Str_file = Str_input_folder;
        Str_file += "/target_states_";
        Str_file += std::to_string(pid);
        Str_file += ".txt";
        create(Str_file);
        writeMember<abs_type>(Str_file, "NO_TARGET_STATES", target_states.size());
        writeSet<abs_type>(Str_file, "SET_TARGET_STATES", target_states);

        delete[] post;

    }
    /* copy other necessary files */
    std::string Str_copy = "cp files/Makefile ";
    Str_folder += "/";
    Str_copy += Str_folder;
    char char_copy[42];
    Length = Str_copy.copy(char_copy, Str_copy.length() + 1);
    char_copy[Length] = '\0';
    system(char_copy);
    Str_copy = "cp files/mutex.cpp ";
    Str_copy += Str_folder;
    Length = Str_copy.copy(char_copy, Str_copy.length() + 1);
    char_copy[Length] = '\0';
    system(char_copy);
    /* create a output folder */
    std::string Str_output_folder = Str_folder;
    Str_output_folder += "Outputs";
    Length = Str_output_folder.copy(char_copy, Str_output_folder.length() + 1);
    char_copy[Length] = '\0';
    checkMakeDir(char_copy);


//    delete[] dl;
//    delete[] mp;
//    delete[] ds;

    return 1;
}
