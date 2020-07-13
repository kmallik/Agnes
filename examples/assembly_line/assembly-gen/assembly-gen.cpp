/*
 * assembly-gen.cpp
 *
 *  Created on: 15.06.2020
 *      author: kaushik
 */

 /*
  * A program to automatically generate the factory assembly line example in a parameterized way.
  * The example was proposed by Ruediger Ehlers, which can be found here: https://bitbucket.org/swenjacobs/syntcomp/src/master/AIGER-Benchmarks/factory_assembly_line/factory_assembly_line.pdf
  *
  */

 #include <stdlib.h>

 // #include "FileHandler.hpp"
 #include "Component.hpp" /* for the definition of the data type abs_type */

 /* The parameters */
 #define ck_ 2
 #define n_ 3
 #define m_ 2

using namespace std;
using namespace negotiation;

/* forward declaration of anonymous functions */
abs_type state_id(abs_type ck, abs_type belt_state_dec, abs_type pos, abs_type limit_pos);
abs_type post_addr(abs_type x, abs_type u, abs_type w);

/*********************************************************/
/* main computation */
/*********************************************************/
int main() {
    /* make a folder for the given parameters */
    std::string Str_folder = "";
    Str_folder += "../assembly_ck_";
    Str_folder += std::to_string(ck_);
    Str_folder += "_n_";
    Str_folder += std::to_string(n_);
    Str_folder += "_m_";
    Str_folder += std::to_string(m_);
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

    /* initialize the array that keeps track of the task status on the conveyer belt */
    bool** task_status_set=new bool*[n_];
    for (int i=0; i<n_; i++) {
        task_status_set[i]=new bool[m_];
    }
    
    /* convert boolean to decimal: the input is given by an array with the MSB being the first array element, and l is the length */
    auto bool2dec = [&](size_t l, bool* binary_array) -> size_t {
        size_t dec=0;
        for (size_t i=0; i<l; i++) {
            dec+=binary_array[l-i-1]*pow(2,i);
        }
        return dec;
    };
    
    /* convert decimal to boolean: the input is a decimal integer and the output is a boolean with the MSB being the first array element, and l being the length of the binary number (length of the returned array) */
    auto dec2bool = [&](size_t dec, size_t& l) -> bool* {
        l=ceil(log2(dec));
        bool* binary_array=new bool[l];
        
        int i = 0;
        while (n > 0) {
            // storing remainder in binary array
            binary_array[i] = n % 2;
            n = n / 2;
            i++;
        }
        return binary_array;
    };
    
    /* The regular states of the two arms is made up of the following variables:
     *  1:  the clock 0,...,ck_-1
     *  2:  task status in the 0-th item encoded in decimal
     *  ...
     *  n_+1: task status in the n_-1-th item encoded in decimal
     *  n_+2: left arm position
     *  n_+3: limit position that the arms are allowed to go to (rightmost position for the left arm and the leftmost position for the right arm).
     *
     * In addition, there is a sink state (index=0) that models violation of the global specification (box leaving the belt with unfinished task for the right arm, and collision for the left arm) */
    size_t no_states = ck_*n_*pow(2,m_)*n_*n_ +1;
    
    /* the id of the states are determined using the following lambda expression when the task_status is given */
    auto state_id = [&](size_t ck, size_t* task_status, size_t pos, size_t limit) -> size_t {
        size_t task_status_id=0;
        /* size of the array task_status is n_ */
        for (int n=0; n<n_; n++) {
            task_status_id+=task_status[n];
        }
        return (ck*pow(2,m_)*pow(n_,3) + task_status_id*pow(n_,2) + pos*n_ + limit + 1);
    };
    
    /* the id of the states are determined using the following lambda expression when the task_status_id is given */
    auto state_id = [&](size_t ck, size_t task_status_id, size_t pos, size_t limit) -> size_t {
        return (ck*pow(2,m_)*pow(n_,3) + task_status_id*pow(n_,2) + pos*n_ + limit + 1);
    };

    /* The control inputs are:
     *  1: Move left
     *  2: Move right
     *  3: Stay and do nothing
     *  4: Stay and perform task 0 on the box underneath
     *  ...
     *  m_+3: Stay and perform task m_ on the box underneath */
    size_t no_control_inputs=m_+3;
    
    /* The disturbance inputs of the left arm, and the outputs of the right arm are:
     *  1: The limit position of the right arm */
    size_t no_dist_inputs_left_arm=n_;
    
    /* The disturbance inputs of the right arm, and the outputs of the left arm are:
     *  1: The limit position of the left arm
     *  2: The number of unfinished tasks in the next right position of the limit position of the left arm */
    size_t no_dist_inputs_right_arm=n_*m_;
    
    /* State-to-output map */
    std::vector<abs_type> state_to_output_left, state_to_output_right;
    /* initialize state_to_output vector */
    for (abs_type i=0; i<no_states; i++) {
        state_to_output_left.push_back(0);
        state_to_output_right.push_back(0);
    }
    state_to_output_left[0]=0; /* the sink state outputs arbitrary value */
    state_to_output_right[0]=0; /* the sink state outputs arbitrary value */
    for (int ck=0; ck<ck_; ck++) {
        size_t loc=0;
        while (loc<n_) {
            size_t task_status_id_local=0;
            while (task_status_id_local<pow(2,m_)) {
                bool* task_status_local=new bool[m_];
                size_t m;
                task_status_local=dec2bool(task_status_id_local,m);
                /* compute the number of finished tasks */
                size_t no_finished_tasks=0;
                for (size_t i=0; i<m; i++) {
                    if (task_status_local[i]) {
                        no_finished_tasks++;
                    }
                }
                for (int pos=0; pos<n_; pos++) {
                    for (int limit=0; limit<n_; limit++) {
                        state_to_output_left[state_id(ck, task_status_id, pos, limit)]= limit*m_+no_finished_tasks;
                        state_to_output_right[state_id(ck, task_status_id, pos, limit)]=limit;
                    }
                }
                task_status_id_local++;
            }
            loc++;
        }
    }
    /* the post arrays */
    std::vector<abs_type>** post_left = new std::vector<abs_type>*[no_states*no_control_inputs*no_dist_inputs_left_arm];
    std::vector<abs_type>** post_right = new std::vector<abs_type>*[no_states*no_control_inputs*no_dist_inputs_right_arm];
    /* initialize post */
    for (size_t i=0; i<no_states; i++) {
        for (size_t j=0; j<no_control_inputs; j++) {
            for (size_t k=0; k<no_dist_inputs_left_arm; k++) {
                std::vector<abs_type>* v1 = new std::vector<abs_type>;
                post_left[j]=v1;
            }
            for (size_t k=0; k<no_dist_inputs_right_arm; k++) {
                std::vector<abs_type>* v2 = new std::vector<abs_type>;
                post_right[j]=v2;
            }
        }
    }
    /* the address of the post vector of the left arm in the "post_left" array is determined using the following lambda expression */
    auto post_left_addr = [&](abs_type x, abs_type u, abs_type w) -> abs_type {
        return (x*no_control_inputs*no_dist_inputs_left_arm + u*no_dist_inputs_left_arm + w);
    };
    /* the address of the post vector of the right arm in the "post_right" array is determined using the following lambda expression */
    auto post_right_addr = [&](abs_type x, abs_type u, abs_type w) -> abs_type {
        return (x*no_control_inputs*no_dist_inputs_right_arm + u*no_dist_inputs_right_arm + w);
    };
    /* add self loops to the sink states */
    for (abs_type u=0; u<no_control_inputs; u++) {
        for (abs_type w=0; w<no_dist_inputs_left_arm; w++) {
            post_left[post_left_addr(i,u,w)]->push_back(i);
        }
        for (abs_type w=0; w<no_dist_inputs_right_arm; w++) {
            post_right[post_right_addr(i,u,w)]->push_back(i);
        }
    }
    /* iterate over all the states to compute the post */
    for (int ck=0; ck<ck_; ck++) {
        size_t loc=0;
        while (loc<n_) {
            size_t task_status_id_local=0;
            while (task_status_id_local<pow(2,m_)) {
                bool* task_status_local=new bool[m_];
                size_t m;
                task_status_local=dec2bool(task_status_id_local,m);
                /* current state */
                size_t curr_state=state_id(ck, task_st)
                for (size_t j=0; j<no_control_inputs; j++) {
                    switch (j) {
                        case 0: /*move left*/
                            <#statements#>
                            break;
                            
                        default:
                            break;
                    }
                    for (size_t k=0; k<no_dist_inputs_left_arm; k++) {
                        <#statements#>
                    }
                }
//                /* compute the number of finished tasks */
//                size_t no_finished_tasks=0;
//                for (size_t i=0; i<m; i++) {
//                    if (task_status_local[i]) {
//                        no_finished_tasks++;
//                    }
//                }
//                for (int pos=0; pos<n_; pos++) {
//                    for (int limit=0; limit<n_; limit++) {
//                        state_to_output_left[state_id(ck, task_status_id, pos, limit)]= limit*m_+no_finished_tasks;
//                        state_to_output_right[state_id(ck, task_status_id, pos, limit)]=limit;
//                    }
//                }
                task_status_id_local++;
            }
            loc++;
        }
    }
    
    
    
    
    
    
    
    
    
    
    for (int pid=0; pid<2; pid++) {
        /* ******** create the processes ******** */
        /* There are states related to the writing process each having two components:
         *   1: the state of one write operation ("idle" (0), "writing" (1))
         *   2: the number of packets remaining to be written (1 to data size)
         *   3: the time remaining in the global timer (deadline to 1)
         *   4: the time remaining until the next packet (max period to 1).
         * In addition, there are three sink states:
         *   1: overall time-out (bad) -> state id 0
         *   2: period time-out (bad) -> state id 1
         *   3: task completed (good) -> state id 2 */
        abs_type no_states = 2*ds[pid]*dl[pid]*mp[pid] + 3;
        /* the id of the writing related states are determined using the following lambda expression */
        auto state_id = [&](abs_type i, abs_type j, abs_type k, abs_type l) -> abs_type {
            if (j==0) {
                /* task completed */
                return 2;
            } else if (k==0) {
                /* overall time-out */
                return 0;
            } else if (l==0) {
                /* period time-out */
                return 1;
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
        for (abs_type i=0; i<no_states; i++) {
            state_to_output.push_back(0);
        }
        /* the three sink states are always idle */
        state_to_output[0]=1;
        state_to_output[1]=1;
        state_to_output[2]=1;
        for (int i=0; i<=1; i++) {
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
                        }
                    }
                }
            }
        }
        /* state labels and state clusters needed for the visualization */
        std::vector<std::string*> state_labels;
        for (abs_type i=0; i<no_states; i++) {
            std::string* s=new std::string;
            *s="";
            state_labels.push_back(s);
        }
        *state_labels[0]="TO";
        *state_labels[1]="period_TO";
        *state_labels[2]="finished";
        std::vector<std::unordered_set<abs_type>*> state_clusters;
        /* the sinks are in one cluster */
        std::unordered_set<abs_type>* set = new std::unordered_set<abs_type>;
        set->insert(0);
        set->insert(1);
        set->insert(2);
        state_clusters.push_back(set);
        for (int j=1; j<=ds[pid]; j++) {
            for (int k=1; k<=dl[pid]; k++) {
                for (int l=1; l<=mp[pid]; l++) {
                    std::unordered_set<abs_type>* this_cluster=new std::unordered_set<abs_type>;
                    for (int i=0; i<=1; i++) {
                        switch (i) {
                            case 0:
                                *state_labels[state_id(i,j,k,l)]+="idle_";
                                break;
                            case 1:
                                *state_labels[state_id(i,j,k,l)]+="writing_";
                                break;
                        }
                        *state_labels[state_id(i,j,k,l)]+=std::to_string(j);
                        *state_labels[state_id(i,j,k,l)]+="_";
                        *state_labels[state_id(i,j,k,l)]+=std::to_string(k);
                        *state_labels[state_id(i,j,k,l)]+="_";
                        *state_labels[state_id(i,j,k,l)]+=std::to_string(l);
                        this_cluster->insert(state_id(i,j,k,l));
                    }
                    state_clusters.push_back(this_cluster);
                }
            }
        }
        /* the post array */
        std::vector<abs_type>** post = new std::vector<abs_type>*[no_states*no_control_inputs*no_dist_inputs];
        /* initialize post */
        /* the sink states */
        for (abs_type i=0; i<3*no_control_inputs*no_dist_inputs; i++) {
            std::vector<abs_type>* v = new std::vector<abs_type>;
            post[i]=v;
        }
        /* the other states */
        int ind=3*no_control_inputs*no_dist_inputs;
        for (int i=0; i<=1; i++) {
            for (int j=1; j<=ds[pid]; j++) {
                for (int k=1; k<=dl[pid]; k++) {
                    for (int l=1; l<=mp[pid]; l++) {
                        for (abs_type u=0; u<no_control_inputs; u++) {
                            for (abs_type w=0; w<no_dist_inputs; w++) {
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
            for (abs_type u=0; u<no_control_inputs; u++) {
                for (abs_type w=0; w<no_dist_inputs; w++) {
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
                    /* state "writing" */
                    x = state_id(1,j,k,l);
                    post[post_addr(x,0,0)]->push_back(state_id(1,j,k-1,l_updated));
                    post[post_addr(x,0,1)]->push_back(state_id(1,j-1,k-1,mp[pid]-1));
                    post[post_addr(x,1,0)]->push_back(state_id(0,j,k-1,l_updated));
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
        writeVec<std::string>(Str_file, "STATE_LABELS", state_labels);
        writeMember<abs_type>(Str_file, "NO_CLUSTERS", state_clusters.size());
        writeVecSet<abs_type>(Str_file, "STATE_CLUSTERS", state_clusters);

        /* ****** create the specifications ******** */
        /* create the safe set: all the states except the time-out states (index 0 and 1) are safe */
        std::unordered_set<abs_type> safe_states;
        for (abs_type i=2; i<no_states; i++) { /* the non time over states are safe */
            safe_states.insert(i);
        }
        // for (abs_type i=0; i<no_states; i++) { /* all the states are safe */
        //     safe_states.insert(i);
        // }
        Str_file = Str_input_folder;
        Str_file += "/safe_states_";
        Str_file += std::to_string(pid);
        Str_file += ".txt";
        create(Str_file);
        writeMember<abs_type>(Str_file, "NO_SAFE_STATES", safe_states.size());
        writeSet<abs_type>(Str_file, "SET_SAFE_STATES", safe_states);
        /* create the target state set for liveness spec */
        std::unordered_set<abs_type> target_states;
        for (abs_type i=0; i<no_states; i++) { /* all the states are in target */
            target_states.insert(i);
        }
        // target_states.insert(2); /* the "task completed" state (index 2) */
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
    Str_copy = "cp files/visualize.sh ";
    Str_copy += Str_output_folder;
    Length = Str_copy.copy(char_copy, Str_copy.length() + 1);
    char_copy[Length] = '\0';
    system(char_copy);


    for (int i=0; i<n_; i++) {
        delete[] task_status_set[i];
    }
    delete[] task_status_set;

    return 1;
}
