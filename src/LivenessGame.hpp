/* LivenessGame.hpp
 *
 *  Created by: Kaushik
 *  Date: 23/12/2019    */

/** @file **/
#ifndef LIVENESSGAME_HPP_
#define LIVENESSGAME_HPP_

#include <vector>
#include <queue>
#include <limits>

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
                throw std::runtime_error("Liveness Game: invalid input.");
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
    /*! compute the set of spoiling behaviors in the form of a safety automaton.
     * \param[in] spoilers          pointer to the safety automaton saving the spoiling behaviors
     * \param[out] out_flag        0 -> some initial states are sure losing, 2 -> all initial states are sure winning, 1-> otherwise. */
    int find_spoilers(negotiation::SafetyAutomaton* spoilers) {
        int out_flag;
//        /* create the spoiler safety automaton */
//        negotiation::SafetyAutomaton spoilers;
        /* solve the liveness game with sure semantics */
        std::vector<unordered_set<abs_type>*> sure_win = solve_liveness_game("sure");
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
        /* solve the liveness game with maybe semantics */
        std::vector<unordered_set<abs_type>*> maybe_win = solve_liveness_game("maybe");
        /* if not all the initial states are maybe winning, then no negotiation is possible: return false */
        bool allInitMaybeWinning=true;
        for (auto i=init_.begin(); i!=init_.end(); ++i) {
            if (maybe_win[*i]->size()==0) {
                allInitMaybeWinning=false;
                break;
            }
        }
        if (!allInitMaybeWinning) {
            out_flag=0;
            return out_flag;
        }
        /* compute the set of states reachable from the intitial states */
        std::unordered_set<abs_type> R = reachable_set();
        /* find intersection of the reachable set with the maybe winning region */
        std::unordered_set<abs_type> W;
        for (auto i=R.begin(); i!=R.end(); ++i) {
            if (maybe_win[*i]->size()!=0) {
                W.insert(*i);
            }
        }
        /* save the post array and number of post elements for possible modification */
        /* number of elements in the post array */
        abs_type L=no_states*no_control_inputs*no_dist_inputs;
        std::unordered_set<abs_type>** post_old = new std::unordered_set<abs_type>*[L];
        for (abs_type l=0; l<L; l++) {
            std::unordered_set<abs_type>* set=new std::unordered_set<abs_type>;
            for (auto i=post[l]->begin(); i!=post[l]->end(); ++i) {
                set->insert(*i);
            }
            post_old[l]=set;
        }
        std::vector<abs_type> no_post_old=no_post;
        /* a vector containing the bad inputs for each state index */
        std::vector<std::unordered_set<abs_type>*> bad_pairs;
        /* compute unsafe pairs as a vector of the same size as the number of states, where the element with index i points to the set of bad disturbance inputs for state i */
        std::vector<std::unordered_set<abs_type>*> unsafe_pairs=find_bad_pairs(W,W);
        /* update the bad pairs */
        for (abs_type i=0; i<no_states; i++) {
            std::unordered_set<abs_type>* set=new std::unordered_set<abs_type>;
            for (auto k=unsafe_pairs[i]->begin(); k!=unsafe_pairs[i]->end(); ++k) {
                set->insert(*k);
            }
            bad_pairs.push_back(set);
        }
        /* update the transition system by removing all the successors of the elements in bad_pairs */
        for (abs_type i=0; i<no_states; i++) {
            for (auto k=bad_pairs[i]->begin(); k!=bad_pairs[i]->end(); ++k) {
                for (abs_type j=0; j<no_control_inputs; j++) {
                    abs_type addr_post = addr_xuw(i,j,*k);
                    post[addr_post]->clear();
                    no_post[addr_post]=0;
                }
            }
        }
//        /* compute the reachable set of states with this updated transition system */
//        R = reachable_set();
        /* save the old target states, as target states will be updated */
        std::unordered_set<abs_type> monitor_target_states_old=monitor_target_states_;
        /* remove those targets which are not in the reachable part of maybe winning region */
        for (auto im=monitor_target_states_.begin(); im!=monitor_target_states_.end(); ++im) {
            if (W.find(*im)==W.end()) {
                monitor_target_states_.erase(*im);
            }
        }
        /* initialize set of states for iteratively computing the LiveLockPairs */
        std::unordered_set<abs_type> T_cur;
        for (auto i=monitor_target_states_.begin(); i!=monitor_target_states_.end(); ++i) {
            T_cur.insert(*i);
        }
        /* repeat until convergence*/
        while (T_cur!=W) {
//            /* save the current targets */
//            T_old=T_cur;
            /* update the monitor target states */
            monitor_target_states_=T_cur;
            /* solve sure reachability game */
            sure_win=solve_reach_avoid_game("sure");
            /* update the current target */
            for (abs_type i=0; i<no_states; i++) {
                if (sure_win[i]->size()!=0) {
                    T_cur.insert(i);
                }
            }
            /* update post: block all control strategies which take the system to the sure losing region */
            for (abs_type i=0; i<no_states; i++) {
                /* only consider the states which are sure winning */
                if (sure_win[i]->size()==0) {
                    continue;
                }
                for (abs_type j=0; j<no_control_inputs; j++) {
                    /* assume that this input is winning */
                    bool winning_input=true;
                    for (abs_type k=0; k<no_dist_inputs; k++) {
                        abs_type addr_post=addr_xuw(i,j,k);
                        for (auto i2=post[addr_post]->begin(); i2!=post[addr_post]->end(); ++i2) {
                            /* if the current post is outside maybe winning region, delete all the posts for all the other disturbance inputs */
                            if (W.find(*i2)==W.end()) {
                                winning_input=false;
                                break;
                            }
                        }
                        if (!winning_input) {
                            break;
                        }
                    }
                    /* if this control input is not winning, clear all the posts for this control input */
                    if (!winning_input) {
                        for (abs_type k2=0; k2<no_dist_inputs; k2++) {
                            abs_type addr_post2=addr_xuw(i,j,k2);
                            post[addr_post2]->clear();
                            no_post[addr_post2]=0;
                        }
                    }
                }
            }
            /* compute live lock pairs */
            /* find complement of T_cur */
            std::unordered_set<abs_type> T_cur_cmp;
            for (abs_type i=0; i<no_states; i++) {
                if (T_cur.find(i)==T_cur.end()) {
                    T_cur_cmp.insert(i);
                }
            }
            std::vector<std::unordered_set<abs_type>*> live_lock_pairs=find_bad_pairs(T_cur_cmp,T_cur);
            /* update bad pairs and post (remove all the transitions caused due to elements in live_lock_pairs) */
            for (abs_type i=0; i<no_states; i++) {
                for (auto k=live_lock_pairs[i]->begin(); k!=live_lock_pairs[i]->end(); ++k) {
                    bad_pairs[i]->insert(*k);
                    for (abs_type j=0; j<no_control_inputs; j++) {
                        abs_type addr_post=addr_xuw(i,j,*k);
                        post[addr_post]->clear();
                        no_post[addr_post]=0;
                    }
                }
            }
        }
        /* find the reachable states using the updated post */
        R = reachable_set();

        /* construct the spoilers safety automaton */
        /* map from new state indices to old state indices: all the losing states (i.e. no maybe winning) are lumped in state 0 */
        std::vector<abs_type> old_state_ind;
//        /* count new states */
//        int no_new_states=0;
        /* the old "reject_G" state is mapped to index 0 */
        old_state_ind.push_back(1);
//        no_new_states++;
        /* the old "reject_A" state is mapped to index 1 */
//        old_state_ind.push_back(0);
//        no_new_states++;
        /* for the rest of the reachable monitor states, a new state index is created, and all the losing and unreachable monitor states are mapped to state 0 */
        for (abs_type i=2; i<no_states; i++) {
            if (R.find(i)!=R.end()) {
                old_state_ind.push_back(i);
//                no_new_states++;
            } //else {
//                old_state_ind.push_back(0);
//            }
        }
        /* construct the full safety automaton capturing the set of spoiling behaviors */
        abs_type no_new_states=old_state_ind.size();
        spoilers->no_states_=no_new_states;
//        spoilers.init_=init_; /* CHECK THIS: ALSO IN SAFETY */
        for (abs_type i=0; i<no_new_states; i++) {
            abs_type q=old_state_ind[i];
            if (init_.find(q)!=init_.end()) {
                spoilers->init_.insert(i);
            }
        }
        spoilers->no_inputs_=no_dist_inputs;
        /* construct the post transition array of the safety automaton */
        std::unordered_set<abs_type>** p=new std::unordered_set<abs_type>*[no_new_states*no_dist_inputs];
        for (abs_type i=0; i<no_new_states*no_dist_inputs; i++) {
            std::unordered_set<abs_type>* v=new std::unordered_set<abs_type>;
            p[i]=v;
        }
        /* first add self loops to the reject state */
        for (abs_type k=0; k<no_dist_inputs; k++) {
            p[addr_xw(0,k)]->insert(0);
        }
        for (abs_type q=1; q<no_new_states; q++) {
            abs_type i=old_state_ind[q];
            for (abs_type k=0; k<no_dist_inputs; k++) {
                abs_type addr_post=addr_xw(q,k);
                if (bad_pairs[i]->find(k)!=bad_pairs[i]->end()) {
                    p[addr_post]->insert(0);
                } else {
                    for (abs_type j=0; j<no_control_inputs; j++) {
                        for (abs_type q2=0; q2<no_new_states; q2++) {
                            abs_type i2=old_state_ind[q2];
                            if (post[addr_xuw(i,j,k)]->find(i2)!=post[addr_xuw(i,j,k)]->end()) {
                                p[addr_post]->insert(q2);
                            }
                        }
                    }
                }
            }
        }
        spoilers->addPost(p);
//        /* write the spoilers automaton to file */
//        spoilers.writeToFile(filename);
        /* restore post, no_post, monitor_target_states_*/
        monitor_target_states_=monitor_target_states_old;
        for (abs_type l=0; l<no_states*no_control_inputs*no_dist_inputs; l++) {
            post[l]->clear();
            for (auto i=post_old[l]->begin(); i!=post_old[l]->end(); ++i) {
                post[l]->insert(*i);
            }
        }
        no_post=no_post_old;
        delete[] p;
        delete[] post_old;

        out_flag=1;
        return out_flag;
    }
    /*! Find pairs of state-disturbance input (x,w1) s.t.:
     *      - x is in W1
     *      - there exists (u,w2) s.t. w1 != w2
     *      - all posts of (x,u,w2) is in W2
     *      - there exists post of (x,u,w1) which is not in W2.
     *
     * \param[in] W1        the set W1
     * \param[in] W2        the set W2
     * \param[out] bad_pairs    the pairs which satisfy the above conditions */
    std::vector<unordered_set<abs_type>*> find_bad_pairs(const unordered_set<abs_type> W1, const unordered_set<abs_type> W2) {
        /* initialize the bad pairs vector */
        std::vector<unordered_set<abs_type>*> bad_pairs;
        for (abs_type i=0; i<no_states; i++) {
            std::unordered_set<abs_type>* set=new std::unordered_set<abs_type>;
            bad_pairs.push_back(set);
        }
        for (abs_type i=0; i<no_states; i++) {
            /* only states in W1 can be in bad pairs */
            if (W1.find(i)==W1.end()) {
                continue;
            }
            for (abs_type j=0; j<no_control_inputs; j++) {
                for (abs_type k=0; k<no_dist_inputs; k++) {
                    /* address in the post array */
                    abs_type addr_post = addr_xuw(i,j,k);
                    for (auto i2=post[addr_post]->begin(); i2!=post[addr_post]->end(); ++i2) {
                        /* if this post is outside W2, then this pair (i,k) could potentially be an unsafe pair */
                        if (W2.find(*i2)==W2.end()) {
                            for (abs_type k2=0; k2<no_dist_inputs; k2++) {
                                if (k==k2) {
                                    continue;
                                }
                                /* address in the post array */
                                abs_type addr_post2 = addr_xuw(i,j,k2);
                                /* the disturbance k2 is friendly if there is at least one successor, and all the successors are in W2 */
                                bool friendly_disturbance;
                                if (post[addr_post2]->size()==0) {
                                    friendly_disturbance=false;
                                } else {
                                    friendly_disturbance=true;
                                    for (auto i3=post[addr_post2]->begin(); i3!=post[addr_post2]->end(); ++i3) {
                                        if (W2.find(*i3)==W2.end()) {
                                            friendly_disturbance=false;
                                            break;
                                        }
                                    }
                                }
                                /* if k2 is friendly, then (i,k) is an unsafe pair */
                                if (friendly_disturbance) {
                                    bad_pairs[i]->insert(k);
                                }
                            }
                        }
                    }
                }
            }
        }
        return bad_pairs;

    }
}; /* end of class definition */
} /* end of namespace negotiation */
#endif
