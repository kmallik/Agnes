/* Spoilers.hpp
 *
 *  Created by: Kaushik
 *  Date: 13/12/2019 */

/** @file **/
#ifndef SPOILERS_HPP_
#define SPOILERS_HPP_

#include <bits/stdc++.h> /* for setting k=highest possible integer by default */

#include "SafetyAutomaton.hpp"

/** @namespace negotiation **/
namespace negotiation {

/**
 *  @class Spoilers
 *
 *  @brief Contains the full and minimized safety automata capturing the spoiling behaviors
 */
class Spoilers {
public:
    /** @brief current level of minimization **/
    int k_;
    /** @brief full set of spoiling behaviors **/
    SafetyAutomaton* spoilers_full_;
    /** @brief minimized spoiling behaviors **/
    SafetyAutomaton* spoilers_mini_;
    /** @brief mapping of states (abstract states to the set of concrete states); quotient[0] = {0} **/
    std::vector<std::unordered_set<abs_type>*> quotient_;
    /** @brief inverse mapping of quotient (concrete states to the abstract states) **/
    std::vector<std::unordered_set<abs_type>*> inv_quotient_;
    /** @brief partitions which are fully refined so far **/
    std::unordered_set<abs_type> refined_partitions_;
public:
    /* constructor */
    Spoilers(SafetyAutomaton* full) {
        /* initially the minimization level is 0 */
        k_=0;
        /* the full safety automaton is given by the argument */
        spoilers_full_= new negotiation::SafetyAutomaton(*full);
        /* the minimized automaton is initialized by clustering all the non-rejecting concrete states into one abstract state and the reject state (index 0) to another */
        spoilers_mini_= new negotiation::SafetyAutomaton();
        spoilers_mini_->no_states_=2;
        /* initially all the initial states of mini are lumped in the state index 1 */
        spoilers_mini_->init_.insert(1);
        spoilers_mini_->no_inputs_=spoilers_full_->no_inputs_;
        /* initialization of quotient and inverse quotient */
        std::unordered_set<abs_type>* s0=new std::unordered_set<abs_type>;
        s0->insert(0);
        quotient_.push_back(s0);
        std::unordered_set<abs_type>* r0=new std::unordered_set<abs_type>;
        r0->insert(0);
        inv_quotient_.push_back(r0);
        std::unordered_set<abs_type>* s1=new std::unordered_set<abs_type>;
        quotient_.push_back(s1);
        for (abs_type i=1; i<spoilers_full_->no_states_; i++) {
            quotient_[1]->insert(i);
            std::unordered_set<abs_type>* r1=new std::unordered_set<abs_type>;
            r1->insert(1);
            inv_quotient_.push_back(r1);
        }
        /* compute the transitions of the minimized automaton */
        computeMiniTransitions();
        /* the partitions which have been refined completely so far */
        refined_partitions_.insert(0);
    }
    /*! Recompute the transitions of the minimized safety automaton from the full safety automaton */
    void computeMiniTransitions() {
        /* create new post array for spoilers_mini_ */
        abs_type ns=spoilers_mini_->no_states_;
        abs_type ni=spoilers_mini_->no_inputs_;
        /* the vector post is used to build the post_ of spoilers_mini_ */
        std::unordered_set<abs_type>** post = new std::unordered_set<abs_type>*[ns*ni];
        for (abs_type i=0; i<ns*ni; i++) {
            std::unordered_set<abs_type>* v=new std::unordered_set<abs_type>;
            post[i]=v;
        }
        /* add transitions to post: j-transitions (j is input) are added between two abstract states qi and ql when there exist j-transitions between some concrete state in qi to some concrete state in ql */
        /* iterate over all the pre concrete states */
        for (abs_type i=0; i<spoilers_full_->no_states_; i++) {
            /* iterate over the corresponding abstract state indices */
            for (auto qi=inv_quotient_[i]->begin(); qi!=inv_quotient_[i]->end(); ++qi) {
                /* iterate over all the inputs */
                for (abs_type j=0; j<spoilers_full_->no_inputs_; j++) {
                    /* iterate over all the concrete post states */
                    for (auto it=spoilers_full_->post_[spoilers_full_->addr(i,j)]->begin(); it!=spoilers_full_->post_[spoilers_full_->addr(i,j)]->end(); ++it) {
                        /* iterate over all the inverse quotients of the concrete post state */
                        for (auto ql=inv_quotient_[*it]->begin(); ql!=inv_quotient_[*it]->end(); ++ql) {
                            post[spoilers_mini_->addr(*qi,j)]->insert(*ql);
                        }
                    }
                }
            }

        }
        /* reset transitions of spoilers_mini_ */
        spoilers_mini_->addPost(post);
        delete[] post;
    }
    /*! Overlapping (S1) and non-overlapping (S2) part of existential predecessor (using the full transition relation) of the minimized state r2 with the minimized state r1
     * \param[in] r1        current abstract state index (group of concrete states)
     * \param[in] r2        post abstract state index (group of concrete states)
     * \param[in] S1        subset of r1 which are in j-predecessor of r2
     * \param[in] S2        subset of r1 which are not in j-predecessor of r2 */
    void computeOverlappingPre(const abs_type& r1, const abs_type& r2, std::unordered_set<abs_type>& S1, std::unordered_set<abs_type>& S2) {
        S1.clear();
        S2.clear();
        for (abs_type j=0; j<spoilers_mini_->no_inputs_; j++) {
            for (std::unordered_set<abs_type>::iterator i=quotient_[r1]->begin(); i!=quotient_[r1]->end(); ++i) {
                for (auto k=spoilers_full_->post_[spoilers_full_->addr(*i,j)]->begin(); k!=spoilers_full_->post_[spoilers_full_->addr(*i,j)]->end(); ++k) {
                    if (quotient_[r2]->find(*k)!=quotient_[r2]->end()) {
                        S1.insert(*i);
                    }
                }
            }
        }
        if (S1.size()<quotient_[r1]->size()) {
            for (auto i=quotient_[r1]->begin(); i!=quotient_[r1]->end(); ++i) {
                if (S1.find(*i)==S1.end()) {
                    S2.insert(*i);
                }
            }
        }
    }
    /*! Refine the abstract states once */
    void refineQuotient() {
        /* S1, S2 are used to store the overlapping and non-overlapping pre states respectively */
        std::unordered_set<abs_type> to_refine, S1, S2;
        /* save the abstract states which have been already refined completely */
        std::unordered_set<abs_type> refined_old = refined_partitions_;
        /* collect all the frontier states which are in 1-step existential predecessor of the already refined states */
        /* iterate over all the abstract states which have not been refined */
        abs_type old_no_states=spoilers_mini_->no_states_;
        for (abs_type i=0; i<old_no_states; i++) {
            if (refined_partitions_.find(i)!=refined_partitions_.end()) {
                continue;
            }
            /* iterate over all the abstract states which have been refined */
            for (std::unordered_set<abs_type>::iterator it=refined_partitions_.begin(); it!=refined_partitions_.end(); ++it) {
                /* compute the set of concrete states in i which are existential predecessors of *it (S1), and the set of states in i which are not predecessor of *it (S2) */
                S1.clear();
                S2.clear();
                computeOverlappingPre(i,*it,S1,S2);
                /* if both S1, S2 are non-empty then i has to be split into two parts */
                if (S1.size()!=0 && S2.size()!=0) {
                    /* S1 is stored in i */
                    quotient_[i]->clear();
                    for (std::unordered_set<abs_type>::iterator k=S1.begin(); k!=S1.end(); ++k) {
                        quotient_[i]->insert(*k);
                        inv_quotient_[*k]->clear();
                        inv_quotient_[*k]->insert(i);
                    }
                    to_refine.insert(i);
                    /* a new abstract state is added */
                    spoilers_mini_->no_states_++;
                    /* S2 is stored in the newly created abstract state */
                    std::unordered_set<abs_type>* S = new std::unordered_set<abs_type>;
                    for (std::unordered_set<abs_type>::iterator k=S2.begin(); k!=S2.end(); ++k) {
                        S->insert(*k);
                        inv_quotient_[*k]->clear();
                        inv_quotient_[*k]->insert(spoilers_mini_->no_states_-1);
                    }
                    quotient_.push_back(S);
                }
            }
        }
        /* if at least some states are to be refined then the index k is incremented */
        if (to_refine.size()!=0) {
            k_++;
        }
        /* refine until no more refinement of the frontier states are possible */
        while (to_refine.size()!=0) {
            /* pop one state from the states to be refined */
            abs_type r = *to_refine.begin();
            to_refine.erase(r);
            /* mark this state as refined */
            refined_partitions_.insert(r);
            /* iterate over all the abstract states */
            for (abs_type i=0; i<spoilers_mini_->no_states_; i++) {
                /* refine the current abstract state r only w.r.t. predecessors of those post states i which have not been refined in the previous round */
                if (refined_old.find(i)==refined_old.end()) {
                    /* compute the set of concrete states in i which are existential predecessors of *it (S1), and the set of states in i which are not predecessor of *it (S2) */
                    S1.clear();
                    S2.clear();
                    computeOverlappingPre(r,i,S1,S2);
                    /* if both S1, S2 are non-empty then r has to be split into two parts */
                    if (S1.size()!=0 && S2.size()!=0) {
                        quotient_[r]->clear();
                        spoilers_mini_->no_states_++;
                        /* the set of all the concrete states in S1 replaces the current index r abstract state */
                        for (std::unordered_set<abs_type>::iterator k=S1.begin(); k!=S1.end(); ++k) {
                            quotient_[r]->insert(*k);
                            inv_quotient_[*k]->clear();
                            inv_quotient_[*k]->insert(r);
                        }
                        /* r could need further refinement */
                        to_refine.insert(r);
                        std::unordered_set<abs_type>* S = new std::unordered_set<abs_type>;
                        /* the set of all the concrete states in S2 which are added as a new abstract state */
                        for (std::unordered_set<abs_type>::iterator k=S2.begin(); k!=S2.end(); ++k) {
                            S->insert(*k);
                            inv_quotient_[*k]->clear();
                            inv_quotient_[*k]->insert(spoilers_mini_->no_states_-1);
                        }
                        quotient_.push_back(S);
                       /* the newly added abstract state could need further refinement */
                        to_refine.insert(spoilers_mini_->no_states_-1);
                       /* the newly added state is marked as refined */
                        refined_old.insert(spoilers_mini_->no_states_-1);
                    }
                }
            }
        }
        /* update the initial states: an abstract state is initial if there is some concrete initial state in it */
        spoilers_mini_->init_.clear();
        for (auto i=spoilers_full_->init_.begin(); i!=spoilers_full_->init_.end(); ++i) {
            for (auto i2=inv_quotient_[*i]->begin(); i2!=inv_quotient_[*i]->end(); ++i2) {
                spoilers_mini_->init_.insert(*i2);
            }
        }
    }
    /*! Perform k-steps of the bounded bisimulation algorithm.
     * \param[in] k         number of refinement iterations */
    void boundedBisim(int k=INT_MAX) {
        for (int i=0; i<k; i++) {
            refineQuotient();
            /* if the refinement didn't produce new partition, then terminate the bounded bisimulation procedure */
            if (k_==i) {
                break;
            }
        }
        spoilers_mini_->resetPost();
        computeMiniTransitions();
//        /* TESTING: BOUNDED BISIM TURNED OFF */
//        spoilers_mini_=spoilers_full_;
//        /* END TESTING  */
    }
}; /* end of class definition */
} /* end of namespace negotiation */
#endif
