/* SafetyGame.hpp
 *
 *  Created by: Kaushik
 *  Date: 02/12/2019 */

/** @file **/
#ifndef SAFETYGAME_HH_
#define SAFETYGAME_HH_

//#include "FileHandler.hpp"

/** @namespace negotiation **/
namespace negotiation {
/**
 *  @class SafetyGame
 *
 *  @brief A header file for solving safety game and creating the spoiling automaton. SafetyGame is a derived class from Monitor.
 */
class SafetyGame: public Monitor {
public:
    /*! constuctor: see <Monitor> **/
    using Monitor::Monitor;
    /*! Solve safety game.
     *
     * The state reject_A (ind=0) is safe but reject_G (ind=1) is unsafe by default.
     *
     * \param[in] component_safe_states         indices of safe states
     * \param[in] str                                                 string specifying the sure/maybe winning condition
     * \param[out] D                             winning state-input pairs */
    std::vector<std::unordered_set<abs_type>*> solve_safety_game(const std::unordered_set<abs_type> component_safe_states, const char* str="sure") {
        /* sanity check */
        if (!strcmp(str,"sure") && !strcmp(str,"maybe")) {
            try {
                throw std::runtime_error("Safety Game: invalid input.");
            } catch (std::exception& e) {
                std::cout << e.what() << "\n";
            }
        }
        /* safe states */
        std::unordered_set<abs_type> monitor_safe_states;
        for (std::unordered_set<abs_type>::iterator it=component_safe_states.begin(); it!=component_safe_states.end(); ++it) {
            for (abs_type j=1; j<no_assume_states; j++) {
                for (abs_type k=1; k<no_guarantee_states; k++) {
                    monitor_safe_states.insert(state_ind(*it,j,k,no_assume_states,no_guarantee_states));
                }
            }
        }
        std::queue<abs_type> Q; /* FIFO queue of bad states */
        std::unordered_set<abs_type> E; /* bad states */
        std::vector<std::unordered_set<abs_type>*> D; /* set of valid inputs indexed by the state indices */
        for (int i=0; i<no_states; i++) {
            std::unordered_set<abs_type>* s = new std::unordered_set<abs_type>;
            D.push_back(s);
        }
        /* state index 0 is safe but 1 is unsafe */
        Q.push(1);
        E.insert(1);
        if (!strcmp(str,"sure")) {
            /* for sure winning, D[i] only contains the winning control inputs from state i */
            for (abs_type i=0; i<no_control_inputs; i++) {
                D[0]->insert(i);
            }
        } else {
            /* for maybe winning, D[i] contains the input indices from the joint control-internal disturbance input space */
            for (abs_type j=0; j<no_control_inputs; j++) {
                for (abs_type k=0; k<no_dist_inputs; k++) {
                    D[0]->insert(joint_action_ind(j,k));
                }
            }
        }
        
        /* initialize Q, E, D for all the states other than 0,1 */
        for (int i=2; i<no_states; i++) {
            if (!isMember<abs_type>(monitor_safe_states,i) || no_post[i]==0) {
                Q.push(i);
                E.insert(i);
            } else {
                if (!strcmp(str,"sure")) {
                    /* for sure winning, D[i] only contains the winning control inputs from state i */
                    for (std::unordered_set<abs_type>::iterator it=valid_input[i]->begin(); it!=valid_input[i]->end(); ++it) {
                        D[i]->insert(*it);
                    }
                } else {
                    /* for maybe winning, D[i] contains the input indices from the joint control-internal disturbance input space */
                    for (std::unordered_set<abs_type>::iterator it=valid_joint_input[i]->begin(); it!=valid_joint_input[i]->end(); ++it) {
                        D[i]->insert(*it);
                    }
                }
            }
        }
        /* iterate until Q is empty, i.e. when a fixed point is reached*/
        while (Q.size()!=0) {
            abs_type x = Q.front();
            Q.pop();
            for (int j=0; j<no_control_inputs; j++) {
                for (int k=0; k<no_dist_inputs; k++) {
                    abs_type x2 = addr_pre(x,j,k);
                    for (std::vector<abs_type>::iterator it=pre[x2]->begin(); it!=pre[x2]->end(); ++it) {
                        /* remove all the control inputs from the pre-states of x which lead to x */
                        if (!strcmp(str,"sure")) {
                            /* for sure winning, remove the control input */
                            D[*it]->erase(j);
                        } else {
                            /* for maybe winning, remove the joint input */
                            D[*it]->erase(joint_action_ind(j,k));
                        }
                        if (D[*it]->size()==0 && !isMember<abs_type>(E,*it)) {
                            Q.push(*it);
                            E.insert(*it);
                        }
                    }
                }
            }
        }
        /* winning strategy */
        return D;
    }
    /*! Generate the spoiling behavior as a safety automaton and write to a file.
     * \param[in] sure_win    maybe winning state-joint input pairs
     * \param[in] maybe_win maybe winning state-control input pairs
     * \param[in] file        filename for storing the safety automaton.
     */
    void find_spoilers(const std::vector<std::unordered_set<negotiation::abs_type>*>& sure_win, const std::vector<std::unordered_set<negotiation::abs_type>*>& maybe_win, const std::string& filename) {
        /* erase the content of the file */
        create(filename);
//        /* get the sure winning states */
//        std::unordered_set<abs_type> sure_win_dom;
//        for (abs_type i=2; i<no_states; i++) {
//            if (sure_win[i]->size()!=0) {
//                sure_win_dom.insert(i);
//            }
//        }
        /* map from old state indices to new state indices */
        std::vector<abs_type> new_state_ind;
        /* count the number of maybe winning states */
        int n=0;
        for (abs_type i=0; i<no_states; i++) {
            if (maybe_win[i]->size()!=0) {
                new_state_ind.push_back(n);
                n++;
            } else {
                new_state_ind.push_back(0);
            }
        }
//        /* the number of states is equal to number of states in the maybe winning domain + 1 (the sink state) */
//        n++;
        writeMember(filename, "NO_STATES", n);
        /* the inputs are just the disturbance inputs */
        writeMember(filename, "NO_INPUTS", no_dist_inputs);
        /* construct the post transition array of the original transition systems */
        std::unordered_set<abs_type>** post=new std::unordered_set<abs_type>*[no_states*no_control_inputs*no_dist_inputs];
        for (abs_type i=0; i<no_states*no_control_inputs*no_dist_inputs; i++) {
            std::unordered_set<abs_type>* v=new std::unordered_set<abs_type>;
            post[i]=v;
        }
        /* first fill the post array by disregarding the control restriction imposed by sure_win and maybe_win */
        /* 0 is the sink state */
        for (abs_type j=0; j<no_control_inputs; j++) {
            for (abs_type k=0; k<no_dist_inputs; k++) {
                post[addr_post(0,j,k)]->insert(0);
            }
        }
        /* for the rest, use the pre */
        for (int i=1; i<no_states; i++) {
            for (abs_type j=0; j<no_control_inputs; j++) {
                for (abs_type k=0; k<no_dist_inputs; k++) {
                    std::vector<abs_type> p=*pre[addr_pre(i,j,k)];
                    for (int l=0; l<p.size(); l++) {
                        post[addr_post(p[l],j,k)]->insert(i);
                    }
                }
            }
        }
        /* next create a new array by disallowing transitions as per the restriction in sure_win and maybe_win, and abstracting away the control inputs */
        std::unordered_set<abs_type>** arr2=new std::unordered_set<abs_type>*[n*no_dist_inputs];
        for (abs_type i=0; i<n*no_dist_inputs; i++) {
            std::unordered_set<abs_type>* v=new std::unordered_set<abs_type>;
            arr2[i]=v;
        }
        /* current state index in the domain of maybe_win */
        int ind=0;
        /* iterate over all the monitor states */
        for (abs_type q=0; q<no_states; q++) {
            /* the current state i is either the reject state, or the q-th state in the domain of maybe_win */
            abs_type i;
            if (maybe_win[q]->size()!=0 || q==0) {
                i=q;
            } else {
                continue;
            }
            /* if the state i is sure winning: no outgoing transition to reject state, and only outgoing transitions conforming to the strategy */
            if (sure_win[i]->size()!=0) {
                /* iterate over all the winning strategies */
                for (std::unordered_set<abs_type>::iterator it=sure_win[i]->begin(); it!=sure_win[i]->end(); ++it) {
                    /* iterate over all the disturbance inputs*/
                    for (abs_type k=0; k<no_dist_inputs; k++) {
                        /* iterate over all the post states */
                        for (std::unordered_set<abs_type>::iterator it2=post[addr_post(i,*it,k)]->begin(); it2!=post[addr_post(i,*it,k)]->end(); it2++) {
                            arr2[addr(ind,k)]->insert(new_state_ind[*it2]);
                        }
                    }
                }
                ind++;
            }
            /* if the state i is maybe (but not sure) winning: outgoing transitions conforming to the strategy, and otherwise to reject states */
            else if (maybe_win[i]->size()!=0) {
                /* iterate over all control inputs */
                for (abs_type j=0; j<no_control_inputs; j++) {
                    /* if all disturbance inputs lead to losing states, then ignore this control input (non-admissible) */
                    bool admissible=false;
                    for (abs_type k=0; k<no_dist_inputs; k++) {
                        if (maybe_win[i]->find(joint_action_ind(j,k)) != maybe_win[i]->end()) {
                            /* the joint action is in the maybe winning strategy */
                            admissible=true;
                            break;
                        }
                    }
                    if (!admissible) {
                        continue;
                    }
                    /* iterate over all the disturbance inputs */
                    for (abs_type k=0; k<no_dist_inputs; k++) {
                        if (maybe_win[i]->find(joint_action_ind(j,k)) != maybe_win[i]->end()) {
                            /* iterate over all the post states */
                            for (std::unordered_set<abs_type>::iterator it2=post[addr_post(i,j,k)]->begin(); it2!=post[addr_post(i,j,k)]->end(); it2++) {
                                arr2[addr(ind,k)]->insert(new_state_ind[*it2]);
                            }
                        } else {
                            /* if the joint action is not in the maybe winning strategy: transition to reject (ind 0) state */
//                            if (maybe_win[i]->find(joint_action_ind(j,k)) == maybe_win[i]->end()) {
                            arr2[addr(ind,k)]->insert(0);
//                            }
                        }
                    }
                }
                ind++;
            }
        }
        /* write arr2 to file */
        writeArrUnorderedSet(filename, "TRANSITION_POST", arr2, n*no_dist_inputs);
 
        delete[] post;
        delete[] arr2;
    }
private:
    /*! Address of post in post array.
     * \param[in] i           state index
     * \param[in] k           disturbance input index
     * \param[out] ind    address of the post state vector in post **/
    inline abs_type addr(const abs_type i, const abs_type k) {
        return (i*no_dist_inputs + k);
    }
    /*! Address of pre in post array.
     * \param[in] i           state index
     * \param[in] j           control input index
     * \param[in] k           disturbance input index
     * \param[out] ind    address of the post state vector in post **/
    inline abs_type addr_post(const abs_type i, const abs_type j, const abs_type k) {
        return (i*no_control_inputs*no_dist_inputs + j*no_dist_inputs + k);
    }
    /*! Index of the disturbance input from a joint control-disturbance index.
     * \param[in] l             joint control-disturbance input index
     * \param[out] j           disturbance input index */
    inline abs_type dist_ind(const abs_type l) {
        return (l % no_dist_inputs);
    }
    
};/* end of class definition */
}/* end of namespace */

#endif
