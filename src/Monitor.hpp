/* Monitor.hpp
 *
 *  Created by: Kaushik
 *  Date: 16/11/2019 */

/** @file **/
#ifndef MONITOR_HH_
#define MONITOR_HH_

#include <vector>
#include <queue>
#include <unordered_set>

#include "Component.hpp" /* for the definition of data types abs_type and abs_ptr_type */

/** @namespace negotiation **/
namespace negotiation {
/**
 *  @class Monitor
 *
 * @brief The product automaton that embeds the assume-guarantee pair in the component transition system.
 *
 * The monitor is made up of finitely many states, and the transitions between the states are labeled with tuples of (control input, disturbance inputs). The transitions can in general be non-deterministic due to external environmental effects.
 * The state 0 is assumed to be the "reject_A" and the state 1 is assumed to be the "reject_G" state.
**/
class Monitor {
public:
    /** @brief number of component states N **/
    abs_type no_states;
    /** @brief number of assume automaton states **/
    abs_type no_assume_states;
    /** @brief number of guarantee automaton states **/
    abs_type no_guarantee_states;
    /** @brief number of control inputs M **/
    abs_type no_control_inputs;
    /** @brief number of internal disturbance inputs P **/
    abs_type no_dist_inputs;
    /** @brief vector containing the list of all pre: pre[i*M*P + j*P + k] lists all pres for the state, control input, dist input pair (i,j,k) */
    std::vector<abs_type>** pre;
//    /** @brief vector[T] containing the list of all pre */
//    std::vector<abs_type> pre;
//    /** @brief vector[N*M*P] containing the pre's address in the array pre[T] **/
//    std::vector<abs_ptr_type> pre_ptr;
//    /** @brief vector[N*M*P] saving the number of pre for each state-control input-disturbance input tuple (i,j,k) **/
//    std::vector<abs_type> no_pre;
    /** @brief vector[N] saving the number of post for each state i **/
    std::vector<abs_type> no_post;
    /** @brief vector[N] saving the set of allowed inputs for each state i **/
    std::vector<std::unordered_set<abs_type>*> valid_input;
public:
    /* copy constructor */
    Monitor(const Monitor& other) {
        no_states=other.no_states;
        no_assume_states=other.no_assume_states;
        no_guarantee_states=other.no_guarantee_states;
        no_control_inputs=other.no_control_inputs;
        no_dist_inputs=other.no_dist_inputs;
        abs_type size = no_states*no_control_inputs*no_dist_inputs;
        pre = new std::vector<abs_type>*[size];
        for (int i=0; i<size; i++) {
            pre[i]=other.pre[i];
        }
//        pre=other.pre;
//        pre_ptr=other.pre_ptr;
//        no_pre=other.no_pre;
        no_post=other.no_post;
        for (int i=0; i<size; i++) {
            valid_input[i]=other.valid_input[i];
        }
    }
    /* Destructor */
    ~Monitor() {
        for (int i=0; i<no_states*no_control_inputs*no_dist_inputs; i++) {
            delete pre[i];
        }
        for (int i=0; i<no_states; i++) {
            delete valid_input[i];
        }
        delete[] pre;
    }
    /* constructor */
    Monitor(Component& comp, SafetyAutomaton& assume, SafetyAutomaton& guarantee) {
        /* sanity check */
        if (comp.no_dist_inputs != assume.no_inputs) {
            try {
                throw std::runtime_error("Monitor: assumption automaton's input alphabet size does not match with the component's disturbance input alphabet's size.");
            } catch (std::exception& e) {
                std::cout << e.what() << "\n";
            }
        }
        if (comp.no_outputs != guarantee.no_inputs) {
            try {
                throw std::runtime_error("Monitor: guarantee automaton's input alphabet size does not match with the component's output alphabet's size.");
            } catch (std::exception& e) {
                std::cout << e.what() << "\n";
            }
        }
        /* first make sure that the assumption and guarantees are deterministic safety automata */
        /* now compute the product */
        no_states=comp.no_states*(assume.no_states-1)*(guarantee.no_states-1)+2;
        no_assume_states=assume.no_states;
        no_guarantee_states=guarantee.no_states;
        no_control_inputs=comp.no_control_inputs;
        no_dist_inputs=comp.no_dist_inputs;
        no_post.assign(no_states,0);
        no_post[0]=no_control_inputs*no_dist_inputs;
        no_post[1]=no_control_inputs*no_dist_inputs;
        pre=new std::vector<abs_type>*[no_states*no_control_inputs*no_dist_inputs];
        for (int i=0; i<no_states*no_control_inputs*no_dist_inputs; i++) {
            std::vector<abs_type> *v=new std::vector<abs_type>;
            pre[i]=v;
        }
        for (int i=0; i<no_states; i++) {
            std::unordered_set<abs_type> *a = new std::unordered_set<abs_type>;
            valid_input.push_back(a);
        }
        for (int ic=0; ic<comp.no_states; ic++) {
            for (int ia=1; ia<no_assume_states; ia++) {
                for (int ig=1; ig<no_guarantee_states; ig++) {
                    for (int j=0; j<no_control_inputs; j++) {
                        for (int k=0; k<no_dist_inputs; k++) {
                            /* the pre state index for the tuple (ic,ia,ig)*/
                            abs_type im = state_ind(ic,ia,ig,no_assume_states,no_guarantee_states);
                            /* the post component states */
                            std::vector<abs_type> ic2 = *comp.post[comp.addr(ic,j,k)];
                            /* the post assumption state (singleton) */
                            abs_type ia2;
                            if (assume.post[assume.addr(ia,k)]->size()==0) {
                                continue;
                            } else {
                                ia2 = (*assume.post[assume.addr(ia,k)])[0];
                            }
                            for (std::vector<abs_type>::iterator it = ic2.begin() ; it != ic2.end(); ++it) {
                                /* the post guarantee states (singleton) */
                                abs_type ig2;
                                if ((guarantee.post[guarantee.addr(ig,comp.state_to_output[*it])])->size()==0) {
                                                   continue;
                                } else {
                                    ig2 = (*guarantee.post[guarantee.addr(ig,comp.state_to_output[*it])])[0];
                                }
                                /* the post state tuple index */
                                abs_type im2 = state_ind(*it,ia2,ig2,no_assume_states,no_guarantee_states);
                                no_post[im]++;
                                valid_input[im]->insert(j);
                                if (ia2!=0 && ig2!=0) {
                                    pre[addr_pre(im2,j,k)]->push_back(im);
                                } else if (ia2==0 && ig2!=0) {
                                    pre[addr_pre(0,j,k)]->push_back(im);
                                } else if (ig2==0) {
                                    pre[addr_pre(1,j,k)]->push_back(im);
                                }
                            }
                        }
                    }
                }
            }
        }
        /* self loops to the reject states */
        for (abs_type j=0; j<no_control_inputs; j++) {
            for (abs_type k=0; k<no_dist_inputs; k++) {
                pre[addr_pre(0,j,k)]->push_back(0);
                pre[addr_pre(1,j,k)]->push_back(1);
            }
        }
    }
    /*! Solve safety game.
     *
     * The state reject_A (ind=0) is safe but reject_G (ind=1) is unsafe by default.
     *
     * \param[in] component_safe_states         indices of safe states
     * \param[out] D                             winning state-input pairs */
    std::vector<std::unordered_set<abs_type>*> solve_safety_game(const std::unordered_set<abs_type> component_safe_states) {
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
        for (abs_type i=0; i<no_control_inputs; i++) {
            D[0]->insert(i);
        }
        /* initialize Q, E, D for all the states other than 0,1 */
        for (int i=2; i<no_states; i++) {
            if (!isMember<abs_type>(monitor_safe_states,i) || no_post[i]==0) {
                Q.push(i);
                E.insert(i);
            } else {
                for (std::unordered_set<abs_type>::iterator it=valid_input[i]->begin(); it!=valid_input[i]->end(); ++it) {
                    D[i]->insert(*it);
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
                        D[*it]->erase(j);
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
    /*! Address of pre in pre array.
     * \param[in] i           state index
     * \param[in] j           control input index
     * \param[in] k           disturbance input index
     * \param[out] ind    address of the post state vector in post **/
    inline abs_type addr_pre(const abs_type i, const abs_type j, const abs_type k) {
        return (i*no_control_inputs*no_dist_inputs + j*no_dist_inputs + k);
    }
    /*! Index of state..
     * \param[in] ic         component state index
     * \param[in] ia         assumption automaton state index, not equal to 0
     * \param[in] ig         guarantee automaton state index, not equal to 0
     * \param[in] na        no. of states of assumption automaton
     * \param[in] ng        no. of states of guarantee automaton
     * \param[out] im       monitor state index **/
    inline abs_type state_ind(const abs_type ic, const abs_type ia, const abs_type ig, const abs_type na, const abs_type ng) {
        /* sanity check */
        if (ia==0) {
            try {
                throw std::domain_error("Monitor:state_ind: The reject state (with index 0) of the assumption automaton should be excluded while constructing the product space of the monitor.\n");
            } catch (std::exception& e) {
                std::cout << e.what();
            }
        }
        if (ig==0) {
            try {
                throw std::domain_error("Monitor:state_ind: The reject state (with index 0) of the guarantee automaton should be excluded while constructing the product space of the monitor.\n");
            } catch (std::exception& e) {
                std::cout << e.what();
            }
        }
        return (ic*(na-1)*(ng-1) + (ia-1)*(ng-1) + (ig-1) + 2); /* the -1 with ia and ig are to shift all the ia and ig indeces leftwards, since the reject state is not used in the product. the +2 in the end is to make sure that reject states of the monitor 0,1 are indeed reserved. */
    }
private:
    /*! Membership querry for an unordered set.
     *  \param[in] S     the unordered set
     *  \param[in] e    the element */
    template<class T>
    bool isMember(const std::unordered_set<T>& S, const T& e) {
        typename std::unordered_set<T>::const_iterator it=S.find(e);
        if (it==S.end()) {
            return false;
        } else {
            return true;
        }
    }
};/* end of class defintions*/
}/* end of namespace negotiation */
#endif
