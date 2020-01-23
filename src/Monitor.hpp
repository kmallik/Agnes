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
    /** @brief number of component states **/
    abs_type no_comp_states;
    /** @brief number of assume automaton states Na **/
    abs_type no_assume_states;
    /** @brief number of guarantee automaton states Ng **/
    abs_type no_guarantee_states;
    /** @brief number of control inputs M **/
    abs_type no_control_inputs;
    /** @brief number of internal disturbance inputs P **/
    abs_type no_dist_inputs;
    /** @brief array containing the list of all pre: pre[i*M*P + j*P + k] lists all pres for the state, control input, dist input pair (i,j,k) */
    std::unordered_set<abs_type>** pre;
    /** @brief array containing the list of all posts: post[i*M*P + j*P + k] lists all posts for the state, control input, dist input pair (i,j,k) */
    std::unordered_set<abs_type>** post;
    /** @brief vector[N*M*P] saving the number of post for each pair (i,j,k) **/
    std::vector<abs_type> no_post;
//    /** @brief vector[N] saving the set of allowed inputs for each state i **/
//    std::vector<std::unordered_set<abs_type>*> valid_input;
//    /** @brief vector[N] saving the set of allowed joint inputs for each state i **/
//    std::vector<std::unordered_set<abs_type>*> valid_joint_input;
public:
    /* copy constructor */
    Monitor(const Monitor& other) {
        no_states=other.no_states;
        init_=other.init_;
        no_comp_states=other.no_comp_states;
        no_assume_states=other.no_assume_states;
        no_guarantee_states=other.no_guarantee_states;
        no_control_inputs=other.no_control_inputs;
        no_dist_inputs=other.no_dist_inputs;
        abs_type size = no_states*no_control_inputs*no_dist_inputs;
        pre = new std::unordered_set<abs_type>*[size];
        for (int i=0; i<size; i++) {
            std::unordered_set<abs_type>* v=new std::unordered_set<abs_type>;
            pre[i]=v;
            for (auto j=other.pre[i]->begin(); j!=other.pre[i]->end(); ++j) {
                pre[i]->insert(*j);
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
//        for (int i=0; i<size; i++) {
//            std::unordered_set<abs_type>* s1 = new std::unordered_set<abs_type>;
//            valid_input.push_back(s1);
//            valid_input[i]=other.valid_input[i];
//            std::unordered_set<abs_type>* s2 = new std::unordered_set<abs_type>;
//            valid_input.push_back(s2);
//            valid_joint_input[i]=other.valid_joint_input[i];
//        }
    }
    /* Destructor */
    ~Monitor() {
//        for (int i=0; i<no_states*no_control_inputs*no_dist_inputs; i++) {
//            delete pre[i];
//            delete post[i];
//        }
//        for (int i=0; i<no_states; i++) {
//            delete valid_input[i];
//            delete valid_joint_input[i];
//        }
        delete[] pre;
        delete[] post;
    }
    /* constructor: the allowed_inputs is a vectory of allowed_inputs of the *monitor states* */
    Monitor(Component& comp, SafetyAutomaton& assume, SafetyAutomaton& guarantee, std::vector<std::unordered_set<abs_type>*>& allowed_control_inputs, std::vector<std::unordered_set<abs_type>*>& allowed_joint_inputs) {
        initialize(comp, assume, guarantee);
        ComputeTransitions(comp, assume, guarantee, allowed_control_inputs, allowed_joint_inputs);
    }
    /* constructor without allowed inputs */
    Monitor(Component& comp, SafetyAutomaton& assume, SafetyAutomaton& guarantee) {
        initialize(comp, assume, guarantee);
        std::vector<std::unordered_set<abs_type>*> allowed_control_inputs, allowed_joint_inputs;
        /* allow all inputs */
        std::unordered_set<abs_type> all_control_inputs, all_joint_inputs;
        for (abs_type j=0; j<comp.no_control_inputs; j++)
            all_control_inputs.insert(j);
        for (abs_type im=0; im<comp.no_states; im++) {
            for (abs_type ia=0; ia<assume.no_states_; ia++) {
                for (abs_type ig=0; ig<guarantee.no_states_; ig++) {
                    std::unordered_set<abs_type>* s = new std::unordered_set<abs_type>;
                    *s=all_control_inputs;
                    allowed_control_inputs.push_back(s);
                }
            }
        }
        for (abs_type j=0; j<comp.no_control_inputs; j++) {
            for (abs_type k=0; k<comp.no_dist_inputs; k++) {
                all_joint_inputs.insert(addr_uw(j,k));
            }
        }
        for (abs_type im=0; im<comp.no_states; im++) {
            for (abs_type ia=0; ia<assume.no_states_; ia++) {
                for (abs_type ig=0; ig<guarantee.no_states_; ig++) {
                    std::unordered_set<abs_type>* s = new std::unordered_set<abs_type>;
                    *s=all_joint_inputs;
                    allowed_joint_inputs.push_back(s);
                }
            }
        }
        ComputeTransitions(comp, assume, guarantee, allowed_control_inputs, allowed_joint_inputs);
    }
    /* initialize all non-transition related members */
    void initialize(Component& comp, SafetyAutomaton& assume, SafetyAutomaton& guarantee) {
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
        no_comp_states=comp.no_states;
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
        // /* initial states of the monitor are given by the cartesian product of the component, assumption initial states, and the gurantee states at time instant 1 (based on the output from the current component state) */
        // for (auto i=comp.init_.begin(); i!=comp.init_.end(); ++i) {
        //     /* the output label of the initial component state is used to initialize the guarantee automaton */
        //     abs_type o=comp.state_to_output[*i];
        //     for (auto k=guarantee.init_.begin(); k!=guarantee.init_.end(); ++k) {
        //         std::unordered_set<abs_type>* p = guarantee.post_[guarantee.addr(*k,o)];
        //         for (auto k2=p->begin(); k2!=p->end(); ++k2) {
        //             for (auto j=assume.init_.begin(); j!=assume.init_.end(); ++j) {
        //                 init_.insert(monitor_state_ind(*i,*j,*k2,no_assume_states,no_guarantee_states));
        //             }
        //         }
        //     }
        // }
        no_control_inputs=comp.no_control_inputs;
        no_dist_inputs=comp.no_dist_inputs;
    }
    /* fill up the pre, post, and no_post arrays */
    void ComputeTransitions(Component& comp, SafetyAutomaton& assume, SafetyAutomaton& guarantee, std::vector<std::unordered_set<abs_type>*>& allowed_control_inputs, std::vector<std::unordered_set<abs_type>*>& allowed_joint_inputs) {
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
        pre=new std::unordered_set<abs_type>*[no_states*no_control_inputs*no_dist_inputs];
        post=new std::unordered_set<abs_type>*[no_states*no_control_inputs*no_dist_inputs];
        for (int i=0; i<no_states*no_control_inputs*no_dist_inputs; i++) {
            std::unordered_set<abs_type> *v=new std::unordered_set<abs_type>;
            pre[i]=v;
            std::unordered_set<abs_type>* s=new std::unordered_set<abs_type>;
            post[i]=s;
        }
//        for (int i=0; i<no_states; i++) {
//            std::unordered_set<abs_type> *a = new std::unordered_set<abs_type>;
//            valid_input.push_back(a);
//            std::unordered_set<abs_type> *b = new std::unordered_set<abs_type>;
//            valid_joint_input.push_back(b);
//        }
        for (int ic=0; ic<comp.no_states; ic++) {
            for (int ia=1; ia<no_assume_states; ia++) {
                for (int ig=1; ig<no_guarantee_states; ig++) {
                    /* the pre state index for the tuple (ic,ia,ig)*/
                    abs_type im = monitor_state_ind(ic,ia,ig,no_assume_states,no_guarantee_states);
                    for (int j=0; j<no_control_inputs; j++) {
                        /* if there is a control strategy, and the current control input is not allowed, then continue with the next one */
                        if ((allowed_control_inputs[im]->size()!=0) &&
                            (allowed_control_inputs[im]->find(j)==allowed_control_inputs[im]->end())) {
                            continue;
                        }
                        for (int k=0; k<no_dist_inputs; k++) {
                            /* if the current joint control input is not allowed, then continue with the next disturbance input */
                            if (allowed_joint_inputs[im]->find(addr_uw(j,k))== allowed_joint_inputs[im]->end()) {
                                continue;
                            }
                            /* if the assume automaton has hit a deadend, then ignore the current disturbance input */
                            if ((assume.post_[assume.addr(ia,k)])->size()==0) {
                                continue;
                            }
                            /* if any of the non-deterministic successors of the assumption automata is rejecting, then this is counted as a rejecting assumption */
                            bool is_assume_reject=false;
                            for (auto ia2=(assume.post_[assume.addr(ia,k)])->begin(); ia2!= (assume.post_[assume.addr(ia,k)])->end(); ++ia2) {
                                if (*ia2==0) {
                                    is_assume_reject=true;
                                    break;
                                }
                            }
                            // /* the post component states */
                            // std::vector<abs_type> ic2 = *comp.post[comp.addr(ic,j,k)];
                            /* non-deterministic post assumption states */
                            for (auto ia2=(assume.post_[assume.addr(ia,k)])->begin(); ia2!= (assume.post_[assume.addr(ia,k)])->end(); ++ia2) {
                                // /* old: the post assumption state (singleton) */
                                // abs_type ia2;
                                // if (assume.post_[assume.addr(ia,k)]->size()==0) {
                                //     continue;
                                // } else {
                                //     ia2 = *(assume.post_[assume.addr(ia,k)])->begin();
                                // }
                                /* non-deterministic component successor states */
                                for (auto ic2 = comp.post[comp.addr(ic,j,k)]->begin() ; ic2 != comp.post[comp.addr(ic,j,k)]->end(); ++ic2) {
                                    // /* old: the post guarantee states (singleton) */
                                    // abs_type ig2;
                                    // if ((guarantee.post_[guarantee.addr(ig,comp.state_to_output[*it])])->size()==0) {
                                    //                    continue;
                                    // } else {
                                    //     ig2 = *(guarantee.post_[guarantee.addr(ig,comp.state_to_output[*it])])->begin();
                                    // }
                                    // /* super old: the guarantee states are updated using the output labels of the component "pre states" */
                                    // if ((guarantee.post_[guarantee.addr(ig,comp.state_to_output[ic])])->size()==0) {
                                    //                    continue;
                                    // } else {
                                    //     ig2 = *(guarantee.post_[guarantee.addr(ig,comp.state_to_output[ic])])->begin();
                                    // }

                                    /* if the guarantee automaton reached a deadend, then ignore the current component successor state */
                                    if ((guarantee.post_[guarantee.addr(ig,comp.state_to_output[*ic2])])->size()==0) {
                                        continue;
                                    }
                                    /* if any of the non-deterministic successors of the guarantee automata is rejecting, then this is counting as a rejecting guarantee */
                                    bool is_guarantee_reject=false;
                                    for (auto ig2=(guarantee.post_[guarantee.addr(ig,comp.state_to_output[*ic2])])->begin(); ig2!=(guarantee.post_[guarantee.addr(ig,comp.state_to_output[*ic2])])->end(); ++ig2) {
                                        if (*ig2==0) {
                                            is_guarantee_reject=true;
                                            break;
                                        }
                                    }
                                    /* if either the assumption or the guarantee hit the bad state, then the monitor goes to one of the sink states and no other transitions are added */
                                    if (is_assume_reject && !is_guarantee_reject) {
                                        if (post[addr_xuw(im,j,k)]->find(0) == post[addr_xuw(im,j,k)]->end()) {
                                            pre[addr_xuw(0,j,k)]->insert(im);
                                            post[addr_xuw(im,j,k)]->insert(0);
                                            no_post[addr_xuw(im,j,k)]++;
                                        }
                                        continue;
                                    // } else if (*ig2==0) {
                                    } else if (is_guarantee_reject) {
                                        if (post[addr_xuw(im,j,k)]->find(1) == post[addr_xuw(im,j,k)]->end()) {
                                            pre[addr_xuw(1,j,k)]->insert(im);
                                            post[addr_xuw(im,j,k)]->insert(1);
                                            no_post[addr_xuw(im,j,k)]++;
                                        }
                                        continue;
                                    }
                                    /* add non-deterministic guarantee successor states */
                                    for (auto ig2=(guarantee.post_[guarantee.addr(ig,comp.state_to_output[*ic2])])->begin(); ig2!=(guarantee.post_[guarantee.addr(ig,comp.state_to_output[*ic2])])->end(); ++ig2) {
                                        /* the post state tuple index */
                                        abs_type im2 = monitor_state_ind(*ic2,*ia2,*ig2,no_assume_states,no_guarantee_states);
                                        if (post[addr_xuw(im,j,k)]->find(im2) == post[addr_xuw(im,j,k)]->end()) {
                                            no_post[addr_xuw(im,j,k)]++;
            //                                valid_input[im]->insert(j);
            //                                valid_joint_input[im]->insert(addr_uw(j,k));
                                            // if (*ia2!=0 && *ig2!=0) {
                                            // if (!is_assume_reject && !is_guarantee_reject) {
                                            pre[addr_xuw(im2,j,k)]->insert(im);
                                            post[addr_xuw(im,j,k)]->insert(im2);
                                            // } else if (*ia2==0 && *ig2!=0) {
                                            // }
                                        }
                                    }
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
                pre[addr_xuw(0,j,k)]->insert(0);
                pre[addr_xuw(1,j,k)]->insert(1);
                post[addr_xuw(0,j,k)]->insert(0);
                post[addr_xuw(1,j,k)]->insert(1);
            }
        }
    }
    /*! Compue the set of states reachable from the initial states */
    std::unordered_set<abs_type> compute_reachable_set() {
        /* the queue of states whose successors are to be explored */
        std::queue<abs_type> fifo;
        /* states already seen */
        std::unordered_set<abs_type> seen;
        /* initialize the queue and seen with the set of initial states */
        for (auto i=init_.begin(); i!=init_.end(); ++i) {
            fifo.push(*i);
            seen.insert(*i);
        }
        /* until no new states are found */
        while (fifo.size()!=0) {
            /* pop the front element */
            abs_type i = fifo.front();
            fifo.pop();
            /* add all the successors of i to the queue */
            for (abs_type j=0; j<no_control_inputs; j++) {
                for (abs_type k=0; k<no_dist_inputs; k++) {
                    /* address in the post array */
                    abs_type post_addr = addr_xuw(i,j,k);
                    for (auto i2=post[post_addr]->begin(); i2!=post[post_addr]->end(); ++i2) {
                        /* if the state i2 is not seen, then add i2 to the queue and seen */
                        if (seen.find(*i2)==seen.end()) {
                            fifo.push(*i2);
                            seen.insert(*i2);
                        }
                    }
                }
            }
        }
        /* the set seen is the set of reachable states */
        return seen;
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
    /*! Index of state-control input pair.
     * \param[in] i           state index
     * \param[in] k           disturbance input index
     * \param[out] ind    address **/
    inline int addr_xw(const abs_type i, const abs_type k) {
        return (i*no_dist_inputs + k);
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
     * \param[in] ia         assumption automaton state index
     * \param[in] ig         guarantee automaton state index
     * \param[in] na        no. of states of assumption automaton
     * \param[in] ng        no. of states of guarantee automaton
     * \param[out] im       monitor state index **/
    inline abs_type monitor_state_ind(const abs_type ic, const abs_type ia, const abs_type ig, const abs_type na, const abs_type ng) {
        // /* sanity check */
        /* violation of guarantee has higher priority than the violation of assumption (strong satisfaction of contract) */
        if (ig==0) {
            return 1;
            // try {
            //     throw std::domain_error("Monitor:monitor_state_ind: The reject state (with index 0) of the guarantee automaton should be excluded while constructing the product space of the monitor.\n");
            // } catch (std::exception& e) {
            //     std::cout << e.what();
            // }
        } else if (ia==0) {
            return 0;
            // try {
            //     throw std::domain_error("Monitor:monitor_state_ind: The reject state (with index 0) of the assumption automaton should be excluded while constructing the product space of the monitor.\n");
            // } catch (std::exception& e) {
            //     std::cout << e.what();
            // }
        } else {
            return (ic*(na-1)*(ng-1) + (ia-1)*(ng-1) + (ig-1) + 2); /* the -1 with ia and ig are to shift all the ia and ig indeces leftwards, since the reject state is not used in the product. the +2 in the end is to make sure that reject states of the monitor 0,1 are indeed reserved. */
        }
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
    /*! (Over-)write the monitor automaton to a file */
    void writeToFile(const string& filename) {
        create(filename);
        writeMember(filename, "NO_STATES", no_states);
        writeMember(filename, "NO_INITIAL_STATES", init_.size());
        writeSet(filename, "INITIAL_STATE_LIST", init_);
        writeMember<abs_type>(filename, "NO_COMP_STATES", no_comp_states);
        writeMember<abs_type>(filename, "NO_ASSUME_STATES", no_assume_states);
        writeMember<abs_type>(filename, "NO_GUARANTEE_STATES", no_guarantee_states);
        writeMember<abs_type>(filename, "NO_CONTROL_INPUTS", no_control_inputs);
        writeMember<abs_type>(filename, "NO_DIST_INPUTS", no_dist_inputs);
        writeArrSet(filename,"TRANSITION_POST",post, no_states*no_control_inputs*no_dist_inputs);
    }
};/* end of class defintions*/
}/* end of namespace negotiation */
#endif
