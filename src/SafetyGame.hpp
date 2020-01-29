/* SafetyGame.hpp
 *
 *  Created by: Kaushik
 *  Date: 02/12/2019 */

/** @file **/
#ifndef SAFETYGAME_HPP_
#define SAFETYGAME_HPP_

#include <cstring>


/** @namespace negotiation **/
namespace negotiation {

using namespace std;
/**
 *  @class SafetyGame
 *
 *  @brief A class for solving safety game and creating the spoiling automaton. SafetyGame is a derived class from Monitor.
 */
class SafetyGame: public Monitor {
public:
//    /*! constuctor: see <Monitor> **/
//    using Monitor::Monitor;
    SafetyGame(Component& comp, SafetyAutomaton& assume, SafetyAutomaton& guarantee) : Monitor(comp, assume, guarantee) {}
    /*! Solve safety game.
     *  The algorithm is taken from: https://gitlab.lrz.de/matthias/SCOTSv0.2/raw/master/manual/manual.pdf
     *
     * The state reject_A (ind=0) is safe but reject_G (ind=1) is unsafe by default.
     *
     * \param[in] component_safe_states         indices of safe states
     * \param[in] str                                                 string specifying the sure/maybe winning condition
     * \param[out] D                             winning (monitor) state-input pairs */
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
        for (auto it=component_safe_states.begin(); it!=component_safe_states.end(); ++it) {
            for (abs_type j=1; j<no_assume_states; j++) {
                for (abs_type k=1; k<no_guarantee_states; k++) {
                    monitor_safe_states.insert(monitor_state_ind(*it,j,k,no_assume_states,no_guarantee_states));
                }
            }
        }
        /* the reject_A state is also safe */
        // /* experimental: "maybe" doesn't consider 0 as safe (nor unsafe), but "sure" does */
        // if (!strcmp(str,"sure")) {
        //     monitor_safe_states.insert(0);
        // }

        monitor_safe_states.insert(0);
        std::queue<abs_type> Q; /* FIFO queue of bad states */
        std::unordered_set<abs_type> E; /* bad states */
        std::vector<std::unordered_set<abs_type>*> D; /* set of valid inputs indexed by the monitor state indices */
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
                    D[0]->insert(addr_uw(j,k));
                }
            }
        }

        /* initialize Q, E, D for all the states other than 0,1 */
        for (int i=2; i<no_states; i++) {
            if (!isMember<abs_type>(monitor_safe_states,i) || isDeadEnd(i)) {
                Q.push(i);
                E.insert(i);
            } else {
                if (!strcmp(str,"sure")) {
                    /* for sure winning, D[i] initially contains all the inputs for which there is some successor */
                    for (abs_type j=0; j<no_control_inputs; j++) {
                        for (abs_type k=0; k<no_dist_inputs; k++) {
                            if (no_post[addr_xuw(i,j,k)]!=0) {
                                D[i]->insert(j);
                                break;
                            }
                        }
                    }
                } else {
                    /* for maybe winning, D[i] initially contains all the joint input indices for which there is some successor */
                    for (abs_type j=0; j<no_control_inputs; j++) {
                        for (abs_type k=0; k<no_dist_inputs; k++) {
                            if (no_post[addr_xuw(i,j,k)]!=0) {
                                D[i]->insert(addr_uw(j,k));
                            }
                        }
                    }
                }
            }
        }
        if (!strcmp(str,"sure")) {
            // /* the set of states from which the game is sure winning (initially only 0) */
            // std::unordered_set<abs_type> W={0};
            /* the set of states from which the game is sure winning unless the assumption is falsified (initially only 0) */
            std::unordered_set<abs_type> W_assumption_frontier={0};
            // /* flag for checking convergence */
            // bool fixpoint_reached=false;
            /* set of friendly disturbance inputs (which lead to assumption falsifying sure winning states) per state */
            std::vector<std::unordered_set<abs_type>*> friendly_dist, friendly_dist_seen;
            for (abs_type i=0; i<no_states; i++) {
                std::unordered_set<abs_type>* set = new std::unordered_set<abs_type>;
                friendly_dist.push_back(set);
                std::unordered_set<abs_type>* set2 = new std::unordered_set<abs_type>;
                friendly_dist_seen.push_back(set2);
            }
            std::queue<abs_type> Q_old=Q; /* saved copy of FIFO queue of bad states */
            std::unordered_set<abs_type> E_old=E; /* saved copy of bad states */
            std::vector<std::unordered_set<abs_type>*> D_old; /* saved copy of set of valid inputs indexed by the monitor state indices */
            for (abs_type i=0; i<no_states; i++) {
                std::unordered_set<abs_type>* set = new std::unordered_set<abs_type>;
                D_old.push_back(set);
                *D_old[i]=*D[i];
            }
            while (1) {
                /* update the list of friendly disturbances */
                std::unordered_set<abs_type> WW=W_assumption_frontier;
                W_assumption_frontier.clear();
                bool fixpoint_reached=true;
                for (auto i=WW.begin(); i!=WW.end(); ++i) {
                    // /* if the current state in the frontier is not in the winning region, then ignore */
                    // if (W.find(*i)==W.end()) {
                    //     continue;
                    // }
                    for (abs_type k=0; k<no_dist_inputs; k++) {
    //                    /* check if the disturbance k surely leads to the already computed winning region W from state i */
    //                    bool sure_winning_dist=true;
                        for (abs_type j=0; j<no_control_inputs; j++) {
                            std::unordered_set<abs_type> p = *pre[addr_xuw(*i,j,k)];
                            for (auto i2=p.begin(); i2!=p.end(); ++i2) {
                                if (friendly_dist_seen[*i2]->find(k)==friendly_dist_seen[*i2]->end()) {
                                    /* if k leads to an unsafe state no matter what control input we choose, then k is not friendly */
                                    bool is_friendly;
                                    for (abs_type j2=0; j2<no_control_inputs; j2++) {
                                        is_friendly=true;
                                        for (auto q=E.begin(); q!=E.end(); ++q) {
                                            if (post[addr_xuw(*i2,j2,k)]->find(*q) != post[addr_xuw(*i2,j2,k)]->end()) {
                                                is_friendly=false;
                                                break;
                                            }
                                        }
                                        if (is_friendly) {
                                            break;
                                        }
                                    }
                                    if (is_friendly) {
                                        friendly_dist[*i2]->insert(k);
                                        W_assumption_frontier.insert(*i2);
                                        friendly_dist_seen[*i2]->insert(k);
                                        fixpoint_reached=false;
                                    }
                                }
    //                            /* if the successor is not in W, then k is not friendly */
    //                            if (W.find(*i2)==W.end()) {
    //                                friendly_dist[i]->insert(k);
    ////                                sure_winning_dist=false;
    //                                break;
    //                            }
                            }
                        }
    //                    if (sure_winning_dist) {
    //                        friendly_dist[i]->insert(k);
    //                    }
                    }
                }
                // /* load the saved value of Q, E and D */
                // Q=Q_old;
                // E=E_old;
                for (abs_type i=0; i<no_states; i++) {
                    *D[i]=*D_old[i];
                }
                /* iterate until Q is empty, i.e. when a fixed point is reached*/
                while (Q.size()!=0) {
                    abs_type x = Q.front();
                    Q.pop();
                    for (int j=0; j<no_control_inputs; j++) {
                        for (int k=0; k<no_dist_inputs; k++) {
                            abs_type x2 = addr_xuw(x,j,k);
                            for (auto it=pre[x2]->begin(); it!=pre[x2]->end(); ++it) {
                                /* the current disturbance input leads to sure winning from the pre state */
        //                        std::unordered_set<abs_type> p = *post[addr_xuw(*it,j,k)];
                                if (friendly_dist[*it]->find(k)==friendly_dist[*it]->end()) {
                                    /* remove all the control inputs from the pre-states of x which lead to x */
//                                            if (!strcmp(str,"sure")) {
                                        /* for sure winning, remove the control input */
                                    D[*it]->erase(j);
//                                            } else {
//                                                /* for maybe winning, remove the joint input */
//                                                D[*it]->erase(addr_uw(j,k));
//                                            }
                                    if (D[*it]->size()==0 && !isMember<abs_type>(E,*it)) {
                                        /* debug */
                                        // std::cout << "state marked as bad = " << *it << "\n";
                                        /* debug end */
                                        Q.push(*it);
                                        E.insert(*it);
                                    }
                                }
                            }
                        }
                    }
                }
                if (fixpoint_reached) {
                    break;
                }
                // /* update the winning region or check if fixpoint is reached */
                // fixpoint_reached=true;
                // for (abs_type i=0; i<no_states; i++) {
                //     if (D[i]->size()==0) {
                //         continue;
                //     // } else if (W.find(i)==W.end()) {
                //     //     W.insert(i);
                //     } else {
                //         fixpoint_reached=false;
                //         break;
                //     }
                // }
                /* Set updates for the next round:
                 *  - Q for the next round contains all the states declared unsafe which were in the assumption violation frontier
                 *  - Clear all the friendly disturbances
                 *  - Remove all the states from assumption violation frontiers which were declared unsafe */
                 /* the original unsafe states (guarantee violation and monitor unsafe states) are always in Q */
                 Q=Q_old;
                for (abs_type i=0; i<no_states; i++) {
                    if (W_assumption_frontier.find(i)!=W_assumption_frontier.end()) {
                        friendly_dist[i]->clear();
                        if (E.find(i)!=E.end()) {
                            Q.push(i);
                            W_assumption_frontier.erase(i);
                        }
                        //  else {
                        //     // W_assumption_frontier.erase(i);
                        // }
                    }
                }
                Q_old=Q;
            }
        } else {
            /* iterate until Q is empty, i.e. when a fixed point is reached*/
            while (Q.size()!=0) {
                abs_type x = Q.front();
                Q.pop();
                for (int j=0; j<no_control_inputs; j++) {
                    for (int k=0; k<no_dist_inputs; k++) {
                        abs_type x2 = addr_xuw(x,j,k);
                        for (auto it=pre[x2]->begin(); it!=pre[x2]->end(); ++it) {
                            /* remove all the joint inputs from the pre-states of x which lead to x */
                            D[*it]->erase(addr_uw(j,k));
                            if (D[*it]->size()==0 && !isMember<abs_type>(E,*it)) {
                                /* debug */
                                // std::cout << "state marked as bad = " << *it << "\n";
                                /* debug end */
                                Q.push(*it);
                                E.insert(*it);
                            }
                        }
                    }
                }
            }
        }




        // /* winning strategy in terms of the component state indices */
        // std::vector<std::unordered_set<abs_type>*> winning_strategy;
        // for (abs_type ic=0; ic<no_comp_states; ic++) {
        //     std::unordered_set<abs_type>* s = new std::unordered_set<abs_type>;
        //     abs_type im = monitor_state_ind(ic,)
        // }
        //
        return D;
    }
    /*! Generate the spoiling behavior as a safety automaton and write to a file.
     * \param[in] sure_win    sure winning state-control input pairs
     * \param[in] maybe_win maybe winning state-join input pairs
     * \param[in] spoilers        the safety automaton storing the spoiling behaviors
     * \param[out] out_flag   0 -> some initial states are sure losing, 2 -> all initial states are sure winning, 1-> otherwise. For out_flag=0,2, spoilers is an automaton that accepts all strings.
     */
    int find_spoilers(const std::vector<std::unordered_set<negotiation::abs_type>*>& sure_win, const std::vector<std::unordered_set<negotiation::abs_type>*>& maybe_win, negotiation::SafetyAutomaton* spoilers) {
        /* the output flag */
        int out_flag;
//        /* cretae the spoiler safety automaton */
//        negotiation::SafetyAutomaton spoilers;
        /* if all the initial states are sure winning, then the set of spoiling behaviors is empty */
        bool allInitSureWinning=true;
        for (auto i=init_.begin(); i!=init_.end(); ++i) {
            if (sure_win[*i]->size()==0) {
                allInitSureWinning=false;
                break;
            }
        }
        if (allInitSureWinning) {
            /* spoilers is a safety automaton that accepts all strings */
            negotiation::SafetyAutomaton safe_all(no_dist_inputs);
            *spoilers = safe_all;
            out_flag=2;
            return out_flag;
        }
        /* if not all the initial states are maybe winning, then no negotiation is possible: return false */
        bool allInitMaybeWinning=true;
        for (auto i=init_.begin(); i!=init_.end(); ++i) {
            if (maybe_win[*i]->size()==0) {
                allInitMaybeWinning=false;
                break;
            }
        }
        if (!allInitMaybeWinning) {
            /* spoilers is a safety automaton that accepts all strings (the game is surely losing, so the other component does not have any extra effect on spoiling the game) */
            negotiation::SafetyAutomaton safe_all(no_dist_inputs);
            *spoilers = safe_all;
            out_flag=0;
            return out_flag;
        }
        /* compute the set of reachable states */
        std::unordered_set<abs_type> reachable_set = compute_reachable_set();
        /* map from old state indices to new state indices: all the losing states (i.e. not maybe winning) and the unreachable states are lumped in state 0 */
        std::vector<abs_type> new_state_ind;
        /* count new states */
        int no_new_states=0;
        /* the "reject_G" state is mapped to index 0 */
        new_state_ind.push_back(1);
        no_new_states++;
        /* the "reject_A" state is mapped to index 1 */
        new_state_ind.push_back(0);
        no_new_states++;
        /* for the rest of the reachable maybe winning monitor states, a new state index is created, and all the losing monitor states are mapped to state 0 */
        for (abs_type i=2; i<no_states; i++) {
            if (maybe_win[i]->size()!=0 && reachable_set.find(i)!=reachable_set.end()) {
                new_state_ind.push_back(no_new_states);
                no_new_states++;
            } else {
                new_state_ind.push_back(0);
            }
        }
        /* construct the full safety automaton capturing the set of spoiling behaviors */
        spoilers->no_states_=no_new_states;
//        spoilers.init_=init_;
        for (auto i=init_.begin(); i!=init_.end(); ++i) {
            spoilers->init_.insert(new_state_ind[*i]);
        }
        spoilers->no_inputs_=no_dist_inputs;
        /* construct the post transition array of the original transition systems */
        std::unordered_set<abs_type>** post_loc=new std::unordered_set<abs_type>*[no_states*no_control_inputs*no_dist_inputs];
        for (abs_type i=0; i<no_states*no_control_inputs*no_dist_inputs; i++) {
            std::unordered_set<abs_type>* v=new std::unordered_set<abs_type>;
            post_loc[i]=v;
        }
        /* first fill the post array by disregarding the control restriction imposed by sure_win and maybe_win */
        /* 0 is the sink state */
        for (abs_type j=0; j<no_control_inputs; j++) {
            for (abs_type k=0; k<no_dist_inputs; k++) {
                post_loc[addr_post(0,j,k)]->insert(0);
            }
        }
        /* for the rest, use the pre */
        for (int i=1; i<no_states; i++) {
            for (abs_type j=0; j<no_control_inputs; j++) {
                for (abs_type k=0; k<no_dist_inputs; k++) {
                    std::unordered_set<abs_type> p=*pre[addr_xuw(i,j,k)];
                    for (auto l=p.begin(); l!=p.end(); ++l) {
                        post_loc[addr_post(*l,j,k)]->insert(i);
                    }
                }
            }
        }
        /* next create a new array by disallowing transitions as per the restriction in sure_win and maybe_win, and abstracting away the control inputs */
        std::unordered_set<abs_type>** arr2=new std::unordered_set<abs_type>*[no_new_states*no_dist_inputs];
        for (abs_type i=0; i<no_new_states*no_dist_inputs; i++) {
            std::unordered_set<abs_type>* v=new std::unordered_set<abs_type>;
            arr2[i]=v;
        }
        /* first add self loops to the reject state */
        for (abs_type k=0; k<no_dist_inputs; k++) {
            arr2[addr(0,k)]->insert(0);
            arr2[addr(1,k)]->insert(1);
        }
        /* current state index in the domain of maybe_win */
        int ind=2;
        /* iterate over all the monitor states */
        for (abs_type q=2; q<no_states; q++) {
            /* the current state i is set as the q-th state in the domain of the reachable part of maybe_win */
            abs_type i;
            if (maybe_win[q]->size()!=0 && reachable_set.find(q)!=reachable_set.end()) {
                i=q;
            } else {
                continue;
            }
            /* if the state i is sure winning: no outgoing transition to reject state, and only outgoing transitions conforming to the strategy */
            if (sure_win[i]->size()!=0) {
                /* iterate over all the winning strategies */
                for (auto it=sure_win[i]->begin(); it!=sure_win[i]->end(); ++it) {
                    /* iterate over all the disturbance inputs*/
                    for (abs_type k=0; k<no_dist_inputs; k++) {
                        /* iterate over all the post states */
                        for (auto it2=post_loc[addr_post(i,*it,k)]->begin(); it2!=post_loc[addr_post(i,*it,k)]->end(); it2++) {
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
                        if (maybe_win[i]->find(addr_uw(j,k)) != maybe_win[i]->end()) {
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
                        if (maybe_win[i]->find(addr_uw(j,k)) != maybe_win[i]->end()) {
                            /* iterate over all the post states */
                            for (auto it2=post_loc[addr_post(i,j,k)]->begin(); it2!=post_loc[addr_post(i,j,k)]->end(); it2++) {
                                arr2[addr(ind,k)]->insert(new_state_ind[*it2]);
                            }
                        } else {
                            /* if the joint action is not in the maybe winning strategy: transition to reject (ind 0) state */
                            arr2[addr(ind,k)]->insert(0);
                        }
                    }
                }
                ind++;
            }
        }
        spoilers->addPost(arr2);
        // debug
       // spoilers->writeToFile("Outputs/safety_spoilers.txt");
        // debug end

        delete[] post_loc;
        delete[] arr2;

        /* successfully generated a spoiling automaton: return out_flag=1 */
        out_flag=1;
        return out_flag;
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
    /*! Check if a given monitor state is a dead-end.
     * \param[in] i     state index*/
    bool isDeadEnd(const abs_type i) {
        for (abs_type j=0; j<no_control_inputs; j++) {
            for (abs_type k=0; k<no_dist_inputs; k++) {
                if (no_post[addr_xuw(i,j,k)]!=0) {
                    return false;
                }
            }
        }
        return true;
    }

};/* end of class definition */
}/* end of namespace */

#endif
