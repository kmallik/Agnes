/* Monitor.hpp
 *
 *  Created by: Kaushik
 *  Date: 16/11/2019 */

/** @file **/
#ifndef MONITOR_HPP_
#define MONITOR_HPP_

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
    /** @brief number of monitor states N **/
    abs_type no_states;
    /** @brief set of initial states **/
    std::unordered_set<abs_type> init_;
    /** @brief number of assume automaton states Na **/
    abs_type no_assume_states;
    /** @brief number of guarantee automaton states Ng **/
    abs_type no_guarantee_states;
    /** @brief number of control inputs M **/
    abs_type no_control_inputs;
    /** @brief number of internal disturbance inputs P **/
    abs_type no_dist_inputs;
    /** @brief array containing the list of all pre: pre[i*M*P + j*P + k] lists all pres for the state, control input, dist input pair (i,j,k) */
    std::vector<abs_type>** pre;
    /** @brief array containing the list of all posts: post[i*M*P + j*P + k] lists all posts for the state, control input, dist input pair (i,j,k) */
    std::unordered_set<abs_type>** post;
    /** @brief vector[N*M*P] saving the number of post for each pair (i,j,k) **/
    std::vector<abs_type> no_post;
    /** @brief vector[N] saving the set of allowed inputs for each state i **/
    std::vector<std::unordered_set<abs_type>*> valid_input;
    /** @brief vector[N] saving the set of allowed joint inputs for each state i **/
    std::vector<std::unordered_set<abs_type>*> valid_joint_input;
public:
    /* copy constructor */
    Monitor(const Monitor& other) {
        no_states=other.no_states;
        init_=other.init_;
        no_assume_states=other.no_assume_states;
        no_guarantee_states=other.no_guarantee_states;
        no_control_inputs=other.no_control_inputs;
        no_dist_inputs=other.no_dist_inputs;
        abs_type size = no_states*no_control_inputs*no_dist_inputs;
        pre = new std::vector<abs_type>*[size];
        for (int i=0; i<size; i++) {
            std::vector<abs_type>* v=new std::vector<abs_type>;
            pre[i]=v;
            for (int j=0; j<other.pre[i]->size(); j++) {
                pre[i]->push_back((*other.pre[i])[j]);
            }
        }
        post = new std::unordered_set<abs_type>*[size];
        for (int i=0; i<size; i++) {
            std::unordered_set<abs_type>* s=new std::unordered_set<abs_type>;
            post[i]=s;
            for (auto j=other.post[i]->begin(); j!=other.post[i]->end(); ++j) {
                post[i]->insert(*j);
            }
        }
        no_post=other.no_post;
        for (int i=0; i<size; i++) {
            valid_input[i]=other.valid_input[i];
            valid_joint_input[i]=other.valid_joint_input[i];
        }
    }
    /* Destructor */
    ~Monitor() {
        for (int i=0; i<no_states*no_control_inputs*no_dist_inputs; i++) {
            delete pre[i];
            delete post[i];
        }
        for (int i=0; i<no_states; i++) {
            delete valid_input[i];
            delete valid_joint_input[i];
        }
        delete[] pre;
        delete[] post;
    }
    /* constructor */
    Monitor(Component& comp, SafetyAutomaton& assume, SafetyAutomaton& guarantee) {
        /* sanity check */
        if (comp.no_dist_inputs != assume.no_inputs_) {
            try {
                throw std::runtime_error("Monitor: assumption automaton's input alphabet size does not match with the component's disturbance input alphabet's size.");
            } catch (std::exception& e) {
                std::cout << e.what() << "\n";
            }
        }
        if (comp.no_outputs != guarantee.no_inputs_) {
            try {
                throw std::runtime_error("Monitor: guarantee automaton's input alphabet size does not match with the component's output alphabet's size.");
            } catch (std::exception& e) {
                std::cout << e.what() << "\n";
            }
        }
        /* compute the product */
       /* number of states is Nc*(Na-1)*(Ng-1)+2 */
        no_states=comp.no_states*(assume.no_states_-1)*(guarantee.no_states_-1)+2;
        no_assume_states=assume.no_states_;
        no_guarantee_states=guarantee.no_states_;
        /* initial states of the monitor are given by the cartesian product of the component, assumption and guarantee initial states */
        for (auto i=comp.init_.begin(); i!=comp.init_.end(); ++i) {
            for (auto j=assume.init_.begin(); j!=assume.init_.end(); ++j) {
                for (auto k=guarantee.init_.begin(); k!=guarantee.init_.end(); ++k) {
                    init_.insert(monitor_state_ind(*i,*j,*k,no_assume_states,no_guarantee_states));
                }
            }
        }
        no_control_inputs=comp.no_control_inputs;
        no_dist_inputs=comp.no_dist_inputs;
        no_post.assign(no_states*no_control_inputs*no_dist_inputs,0);
        /* the reject states are sink states */
        for (abs_type i=0; i<=1; i++) {
            for (abs_type j=0; j<no_control_inputs; j++) {
                for (abs_type k=0; k<no_dist_inputs; k++) {
                    no_post[addr_xuw(i,j,k)]=1;
                }
            }
        }
        /* compute and store the predecessors, successors, valid inputs, and valid joint inputs for fast synthesis */
        pre=new std::vector<abs_type>*[no_states*no_control_inputs*no_dist_inputs];
        post=new std::unordered_set<abs_type>*[no_states*no_control_inputs*no_dist_inputs];
        for (int i=0; i<no_states*no_control_inputs*no_dist_inputs; i++) {
            std::vector<abs_type> *v=new std::vector<abs_type>;
            pre[i]=v;
            std::unordered_set<abs_type>* s=new std::unordered_set<abs_type>;
            post[i]=s;
        }
        for (int i=0; i<no_states; i++) {
            std::unordered_set<abs_type> *a = new std::unordered_set<abs_type>;
            valid_input.push_back(a);
            std::unordered_set<abs_type> *b = new std::unordered_set<abs_type>;
            valid_joint_input.push_back(b);
        }
        for (int ic=0; ic<comp.no_states; ic++) {
            for (int ia=1; ia<no_assume_states; ia++) {
                for (int ig=1; ig<no_guarantee_states; ig++) {
                    for (int j=0; j<no_control_inputs; j++) {
                        for (int k=0; k<no_dist_inputs; k++) {
                            /* the pre state index for the tuple (ic,ia,ig)*/
                            abs_type im = monitor_state_ind(ic,ia,ig,no_assume_states,no_guarantee_states);
                            /* the post component states */
                            std::vector<abs_type> ic2 = *comp.post[comp.addr(ic,j,k)];
                            /* the post assumption state (singleton) */
                            abs_type ia2;
                            if (assume.post_[assume.addr(ia,k)]->size()==0) {
                                continue;
                            } else {
                                ia2 = *(assume.post_[assume.addr(ia,k)])->begin();
                            }
                            for (auto it = ic2.begin() ; it != ic2.end(); ++it) {
                                /* the post guarantee states (singleton) */
                                abs_type ig2;
                                if ((guarantee.post_[guarantee.addr(ig,comp.state_to_output[*it])])->size()==0) {
                                                   continue;
                                } else {
                                    ig2 = *(guarantee.post_[guarantee.addr(ig,comp.state_to_output[*it])])->begin();
                                }
                                /* the post state tuple index */
                                abs_type im2 = monitor_state_ind(*it,ia2,ig2,no_assume_states,no_guarantee_states);
                                no_post[addr_xuw(im,j,k)]++;
                                valid_input[im]->insert(j);
                                valid_joint_input[im]->insert(addr_uw(j,k));
                                if (ia2!=0 && ig2!=0) {
                                    pre[addr_xuw(im2,j,k)]->push_back(im);
                                    post[addr_xuw(im,j,k)]->insert(im2);
                                } else if (ia2==0 && ig2!=0) {
                                    pre[addr_xuw(0,j,k)]->push_back(im);
                                    post[addr_xuw(im,j,k)]->insert(0);
                                } else if (ig2==0) {
                                    pre[addr_xuw(1,j,k)]->push_back(im);
                                    post[addr_xuw(im,j,k)]->insert(1);
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
                pre[addr_xuw(0,j,k)]->push_back(0);
                pre[addr_xuw(1,j,k)]->push_back(1);
                post[addr_xuw(0,j,k)]->insert(0);
                post[addr_xuw(1,j,k)]->insert(1);
            }
        }
    }
    /*! Index of state-control input-disturbance input pair.
     * \param[in] i           state index
     * \param[in] j           control input index
     * \param[in] k           disturbance input index
     * \param[out] ind    address of the post state vector in post **/
    inline abs_type addr_xuw(const abs_type i, const abs_type j, const abs_type k) {
        return (i*no_control_inputs*no_dist_inputs + j*no_dist_inputs + k);
    }
    /*! Joint control action-disturbance input index.
     * \param[in] j           control input index
     * \param[in] k           disturbance input index
     * \param[out] l         index of action-pair (j,k) **/
    inline abs_type addr_uw(const abs_type j, const abs_type k) {
        return (j*no_dist_inputs + k);
    }
    /*! Index of state-control input pair.
     * \param[in] i           state index
     * \param[in] j           input index
     * \param[out] ind    address **/
    inline int addr_xu(const abs_type i, const abs_type j) {
        return (i*no_control_inputs + j);
    }
    /*! Index of monitor state from component state
     * \param[in] im         component state index
     * \param[out] ic       component state index **/
    inline abs_type component_state_ind(const abs_type im) {
        /* sanity check */
        if (im>=no_states) {
            try {
                throw std::domain_error("Monitor::component_state_ind: monitor state index out of bound.\n");
            } catch (std::exception& e) {
                std::cout << e.what();
            }
        }
        if (im==0 || im==1) {
            try {
                throw std::domain_error("Monitor::component_state_ind: the monitor reject states do not correspond to any component state.\n");
            } catch (std::exception& e) {
                std::cout << e.what();
            }
        }
        return (im-2)/(no_assume_states-1);
    }
    /*! Index of component state from monitor state
     * \param[in] ic         component state index
     * \param[in] ia         assumption automaton state index, not equal to 0
     * \param[in] ig         guarantee automaton state index, not equal to 0
     * \param[in] na        no. of states of assumption automaton
     * \param[in] ng        no. of states of guarantee automaton
     * \param[out] im       monitor state index **/
    inline abs_type monitor_state_ind(const abs_type ic, const abs_type ia, const abs_type ig, const abs_type na, const abs_type ng) {
        /* sanity check */
        if (ia==0) {
            try {
                throw std::domain_error("Monitor:monitor_state_ind: The reject state (with index 0) of the assumption automaton should be excluded while constructing the product space of the monitor.\n");
            } catch (std::exception& e) {
                std::cout << e.what();
            }
        }
        if (ig==0) {
            try {
                throw std::domain_error("Monitor:monitor_state_ind: The reject state (with index 0) of the guarantee automaton should be excluded while constructing the product space of the monitor.\n");
            } catch (std::exception& e) {
                std::cout << e.what();
            }
        }
        return (ic*(na-1)*(ng-1) + (ia-1)*(ng-1) + (ig-1) + 2); /* the -1 with ia and ig are to shift all the ia and ig indeces leftwards, since the reject state is not used in the product. the +2 in the end is to make sure that reject states of the monitor 0,1 are indeed reserved. */
    }
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
