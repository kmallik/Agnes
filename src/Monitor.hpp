/* Monitor.hpp
 *
 *  Created by: Kaushik
 *  Date: 16/11/2019 */

/** @file **/
#ifndef MONITOR_HH_
#define MONITOR_HH_

#include <vector>

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
    /** @brief number of control inputs M **/
    abs_type no_control_inputs;
    /** @brief number of internal disturbance inputs P **/
    abs_type no_dist_inputs;
    /** @brief vector containing the list of all pre: pre[(i-1)*M*P + (j-1)*P + k] lists all pres for the state, control input, dist input pair (i,j,k) */
    std::vector<abs_type>** pre;
//    /** @brief vector[T] containing the list of all pre */
//    std::vector<abs_type> pre;
//    /** @brief vector[N*M*P] containing the pre's address in the array pre[T] **/
//    std::vector<abs_ptr_type> pre_ptr;
//    /** @brief vector[N*M*P] saving the number of pre for each state-control input-disturbance input tuple (i,j,k) **/
//    std::vector<abs_type> no_pre;
//    /** @brief vector[N*M*P] saving the number of post for each state-control input-disturbance input tuple (i,j,k) **/
//    std::vector<abs_type> no_post;
public:
    /* copy constructor */
    Monitor(const Monitor& other) {
        no_states=other.no_states;
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
//        no_post=other.no_post;
    }
    /* constructor */
    Monitor(const Component& comp, const SafetyAutomaton& assume, const SafetyAutomaton& guarantee) {
        /* first make sure that the assumption and guarantees are deterministic safety automata */
        /* now compute the product */
        no_states=(comp.no_states-1)*(assume.no_states-1)*(guarantee.no_states-1)+2;
        no_control_inputs=comp.no_control_inputs;
        no_dist_inputs=comp.no_dist_inputs;
        pre=new std::vector<abs_type>*[no_states*no_control_inputs*no_dist_inputs];
        for (int ic=0; ic<comp.no_states; ic++) {
            for (int ia=0; ia<assume.no_states; ia++) {
                for (int ig=0; ig<guarantee.no_states; ig++) {
                    for (int j=0; j<no_control_inputs; j++) {
                        for (int k=0; k<no_dist_inputs; k++) {
                            /* the pre state index for the tuple (ic,ia,ig)*/
                            abs_type im = state_ind(ic,ia,ig,assume.no_states,guarantee.no_states);
                            /* self loops to the reject states */
                            if (im==0 || im==1) {
                                pre[addr_pre(im,j,k)].push_back(im);
                            }
                            /* the post component state */
                            std::vector<abs_type> ic2 = *comp.post[addr(ic,j,k)];
                            /* the post assumption state (singleton) */
                            abs_type ia2 = (*assume.post[addr(ia,k)])[0];
                            /* the post guarantee state (singleton) */
                            abs_type ig2 = (*guarantee.post[addr(ig,comp.state_to_output[ic2])])[0];
                            /* the post state tuple index */
                            abs_type im2 = state_ind(ic2,ia2,ig2,assume.no_states,guarantee.no_states);
                            if (ia2!=0 && ig2!=0) {
                                pre[addr_pre(im2,j,k)]->push_back(im);
                            } else if (ia2==0 && ig2!=0) {
                                pre[addr_pre(0,j,k)].push_back(im);
                            } else if (ig2==0) {
                                pre[addr_pre(1,j,k)].push_back(im);
                            }
                        }
                    }
                }
            }
        }
    }
    /*! Address of pre in pre array.
     * \param[in] i           state index
     * \param[in] j           control input index
     * \param[in] k           disturbance input index
     * \param[out] ind    address of the post state vector in post **/
    inline int addr_pre(const int i, const int j, const int k) {
        return ((i-1)*no_control_inputs*no_dist_inputs + (j-1)*no_dist_inputs + k);
    }
    /*! Index of state..
     * \param[in] ic         component state index
     * \param[in] ia         assumption automaton state index
     * \param[in] ig         guarantee automaton state index
     * \param[in] na        no. of states of assumption automaton
     * \param[in] ng        no. of states of guarantee automaton
     * \param[out] im       monitor state index **/
    inline int state_ind(const int ic, const int ia, const int ig, const int na, const int ng) {
        return ((ic-1)*na*ng + (ia-1)*ng + ig);
    }
};/* end of class defintions*/
}/* end of namespace negotiation */
#endif
