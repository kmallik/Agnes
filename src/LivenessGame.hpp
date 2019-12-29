/* LivenessGame.hpp
 *
 *  Created by: Kaushik
 *  Date: 23/12/2019    */

/** @file **/
#ifndef LIVENESSGAME_HPP_
#define LIVENESSGAME_HPP_

#include <vector>
#include <queue>

#include "Component.hpp" /* for the definition of data types abs_type and abs_ptr_type */

/** @namespace negotiation **/
namespace negotiation {

/**
 *  @class LivenessGame
 *
 * @brief A class for solving liveness game and creating the spoiling automaton. LivenessGame is a derived class from Monitor.
*/
class LivenessGame: public Monitor {
public:
    /*! Target states for the liveness/reachability specification */
    std::unordered_set<abs_type> monitor_target_states_;
    /*! Obstacle states for the liveness/reachability specification */
    std::unordered_set<abs_type> monitor_avoid_states_;
    /*! Allowed control inputs */
    std::vector<std::unordered_set<abs_type>*> allowed_inputs_;
//    /*! constuctor: see <Monitor> **/
//    using Monitor::Monitor;
    /*! constructor
     *
     * \param[in] comp          the component
     * \param[in] assume      the assumption safety automaton
     * \param[in] guarantee the guarantee safety automaton
     * \param[in] component_target_states   set of component target states
     * \param[in] allowed_inputs    vector of allowed inputs indexed using the component state indices */
    LivenessGame(Component& comp, SafetyAutomaton& assume, SafetyAutomaton& guarantee, const std::unordered_set<abs_type> component_target_states, std::vector<std::unordered_set<abs_type>*> allowed_inputs) : Monitor(comp, assume, guarantee) {
        /* target states */
        /* the assumption violation is always in target */
        monitor_target_states_.insert(0);
        /* derive the other target states from the component, assumption, and guarantee state indices */
        for (auto it=component_target_states.begin(); it!=component_target_states.end(); ++it) {
            for (abs_type j=1; j<no_assume_states; j++) {
                for (abs_type k=1; k<no_guarantee_states; k++) {
                    monitor_target_states_.insert(monitor_state_ind(*it,j,k,no_assume_states,no_guarantee_states));
                }
            }
        }
        /* avoid state: the guarantee violation */
        monitor_avoid_states_.insert(1);
        /* allowed inputs */
        for (abs_type im=0; im<no_states; im++) {
            std::unordered_set<abs_type>* s=new std::unordered_set<abs_type>;
            /* for the sink, all inputs are allowed, and for the other states, the allowed inputs are derived from the allowed inputs for the respective component state */
            if (im==0 || im==1) {
                for (abs_type j=0; j<no_control_inputs; j++) {
                    s->insert(j);
                }
            } else {
                abs_type ic=component_state_ind(im);
                for (auto l=allowed_inputs[ic]->begin(); l!=allowed_inputs[ic]->end(); ++l) {
                    s->insert(*l);
                }
            }
            allowed_inputs_.push_back(s);
        }
    }
    /*! Solve reach-avoid game, where the target is given by the local specification, and the "obstacle" is given by the reject_G (violation of guarantee) state.
     *  The algorithm is taken from: https://gitlab.lrz.de/matthias/SCOTSv0.2/raw/master/manual/manual.pdf
     *
     * The state reject_A (ind=0) is safe but reject_G (ind=1) is unsafe by default.
     *
     * \param[in] str                                                  string specifying the sure/maybe winning condition
     * \param[out] D                                                    optimal state-input pairs */
    std::vector<unordered_set<abs_type>*> solve_reach_avoid_game(const char* str="sure") {
        /* sanity check */
        if (!strcmp(str,"sure") && !strcmp(str,"maybe")) {
            try {
                throw std::runtime_error("Reach-avoid Game: invalid input.");
            } catch (std::exception& e) {
                std::cout << e.what() << "\n";
            }
        }
        /* a very high number used as value of losing states */
        abs_type losing = std::numeric_limits<abs_type>::max();
        /* FIFO queue */
        std::queue<abs_type> Q;
        for (auto i=monitor_target_states_.begin(); i!=monitor_target_states_.end(); ++i) {
            Q.push(*i);
        }
        /* value function */
        std::vector<abs_type> V;
        /* intermediate values of the state input pairs */
        std::vector<abs_type> M;
        for (abs_type i=0; i<no_states; i++) {
            for (abs_type j=0; j<no_control_inputs; j++) {
                if (!strcmp(str,"sure")) { /* for sure winning, the values are associated with state-control input pairs*/
                    M.push_back(0);
                } else { /* for maybe winning, the values are associated with state-control input-disturbance input triples */
                    for (abs_type k=0; k<no_dist_inputs; k++) {
                        M.push_back(0);
                    }
                }
            }
        }
        /* bookkeeping of processed states */
        std::unordered_set<abs_type> E;
        /* optimal inputs indexed by the state indices */
        std::vector<std::unordered_set<abs_type>*> D;
        /* initialize the values of the non-target states with "losing", and the optimal inputs of the target states with the input 0 (chosen arbitrarily, can be anything). */
        for (abs_type i=0; i<no_states; i++) {
            std::unordered_set<abs_type>* s=new std::unordered_set<abs_type>;
            if (monitor_target_states_.find(i)!=monitor_target_states_.end()) { /* i is in target set */
                V.push_back(0); /* value is 0 */
                s->insert(0); /* any input is optimal, 0 is chosen arbitrarily */
                D.push_back(s);
            } else { /* i is not in target */
                V.push_back(losing);
                D.push_back(s);
            }
        }
        /* keep track of the number of processed posts */
        std::vector<abs_type> K;
        for (abs_type i=0; i<no_states; i++) {
            for (abs_type j=0; j<no_control_inputs; j++) {
                abs_type np=0;
                for (abs_type k=0; k<no_dist_inputs; k++) {
                    if (!strcmp(str,"maybe")) {
                        K.push_back(no_post[addr_xuw(i,j,k)]);
                    } else {
                        np+=no_post[addr_xuw(i,j,k)];
                    }
                }
                K.push_back(np);
            }
        }
        /* until the queue is empty */
        while (Q.size()!=0) {
            /* pop the oldest element */
            abs_type x = Q.front();
            Q.pop();
            /* update the set of seen states */
            E.insert(x);
            for (abs_type j=0; j<no_control_inputs; j++) {
                for (abs_type k=0; k<no_dist_inputs; k++) {
                    abs_type x2 = addr_xuw(x,j,k);
                    for (auto it=pre[x2]->begin(); it!=pre[x2]->end(); ++it) {
                        /* if the pre state is in avoid, ignore */
                        if (monitor_avoid_states_.find(*it)!=monitor_avoid_states_.end()) {
                            continue;
                        }
                        /* check the viability of the used control input */
                        if (allowed_inputs_[*it]->find(j)==allowed_inputs_[*it]->end()) {
                            continue;
                        }
                        if (!strcmp(str,"sure")) {
                            K[addr_xu(*it,j)]--;
                            M[addr_xu(*it,j)] = (M[addr_xu(*it,j)]>=1+V[x] ? M[addr_xu(*it,j)] : 1+V[x]);
                            if (!K[addr_xu(*it,j)] && V[*it]>M[addr_xu(*it,j)]) {
                                Q.push(*it);
                                V[*it]=M[addr_xu(*it,j)];
                                D[*it]->clear();
                                D[*it]->insert(j);
                            }
                        } else {
                            K[addr_xuw(*it,j,k)]--;
                            M[addr_xuw(*it,j,k)] = (M[addr_xuw(*it,j,k)]>=1+V[x] ? M[addr_xuw(*it,j,k)] : 1+V[x]);
                            if (!K[addr_xuw(*it,j,k)] && V[*it]>M[addr_xuw(*it,j,k)]) {
                                Q.push(*it);
                                V[*it]=M[addr_xuw(*it,j,k)];
                                D[*it]->clear();
                                D[*it]->insert(addr_uw(j,k));
                            }
                        }
                    }
                }
            }
        }
        return D;
    }
    /*! Solve Buchi game with additional safety objective.
     *
     *  \param[in] str                                                  string specifying the sure/maybe winning condition
     *  \param[out] D                                                    optimal state-input pairs */
    std::vector<unordered_set<abs_type>*> solve_liveness_game(const char* str="sure") {
        /* sanity check */
        if (!strcmp(str,"sure") && !strcmp(str,"maybe")) {
            try {
                throw std::runtime_error("Buchi-avoid Game: invalid input.");
            } catch (std::exception& e) {
                std::cout << e.what() << "\n";
            }
        }
//        initialize(component_target_states);
        /* a very high number used as value of losing states */
        abs_type losing = std::numeric_limits<abs_type>::max();
        /* the outer nu variable */
        std::unordered_set<abs_type> YY, YY_old;
        for (abs_type i=0; i<no_states; i++) {
            YY.insert(i);
        }
        /* set of allowed inputs (control input when str=sure, joint input when str=maybe) indexed by the state indices */
        std::vector<std::unordered_set<abs_type>*> D;
        for (abs_type i=0; i<no_states; i++) {
            std::unordered_set<abs_type>* s = new std::unordered_set<abs_type>;
            D.push_back(s);
            for (auto j=allowed_inputs_[i]->begin(); j!=allowed_inputs_[i]->end(); ++j) {
                if (!strcmp(str,"sure")) {
                    D[i]->insert(*j);
                } else {
                    for (abs_type k=0; k<no_dist_inputs; k++) {
                        D[i]->insert(addr_uw(*j,k));
                    }
                }
            }
        }
        /* the inner mu variable */
        std::unordered_set<abs_type> XX;
        std::vector<unordered_set<abs_type>*> reach_win;
        std::unordered_set<abs_type> safe_targets;
        /* iterate until a fix-point of YY is reached */
        while (YY_old.size()!=YY.size()) {
            /* save the current YY */
            YY_old=YY;
            /* the set of targets from which it is possible to stay in YY in the next step */
//            std::unordered_set<abs_type> safe_targets;
            safe_targets.clear();
            for (auto i=monitor_target_states_.begin(); i!=monitor_target_states_.end(); ++i) {
                /* only consider i from the intersection of YY and target */
                if (YY.find(*i)==YY.end()) {
                    continue;
                }
                for (abs_type j=0; j<no_control_inputs; j++) {
                    for (abs_type k=0; k<no_dist_inputs; k++) {
                        /* the address to look up in the post array */
                        abs_type l=addr_xuw(*i,j,k);
                        for (auto i2=post[l]->begin(); i2!=post[l]->end(); ++i2) {
                            /* if the successor i2 is not in YY, then the corresponding state-action pair is unsafe */
                            if (YY.find(*i2)==YY.end()) {
                                if (!strcmp(str,"sure")) {
                                    D[*i]->erase(j);
                                } else {
                                    D[*i]->erase(addr_uw(j,k));
                                }
                                continue;
                            }
                        }
                    }
                }
                /* the safe targets are those which have some action that makes sure that the successor is in YY */
                if (D[*i]->size()!=0) {
                    safe_targets.insert(*i);
                }
            }
            /* create a new monitor with safe_targets as the true targets */
            negotiation::LivenessGame monitor2(*this);
            monitor2.monitor_target_states_=safe_targets;
            /* solve reach_avoid_game on monitor2 */
            reach_win.clear();
            reach_win=monitor2.solve_reach_avoid_game(str);
            /* create a vector of the winning states in the reach_avoid game */
            XX.clear();
            for (abs_type i=0; i<no_states; i++) {
                if (reach_win[i]->size()!=0) {
                    XX.insert(i);
                }
            }
            YY=XX;
        }
        return reach_win;
    }
}; /* end of class definition */
} /* end of namespace negotiation */
#endif
