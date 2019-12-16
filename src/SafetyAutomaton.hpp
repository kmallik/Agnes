/* SafetyAutomaton.hpp
 *
 *  Created by: Kaushik
 *  Date: 16/11/2019  */

/** @file **/
#ifndef SAFETYAUTOMATON_HPP_
#define SAFETYAUTOMATON_HPP_

#include <vector>

//#include "FileHandler.hpp"
#include "Component.hpp" /* for the definition of data types abs_type and abs_ptr_type */

/** @namespace negotiation **/
namespace negotiation {

/**
 *  @class SafetyAutomaton
 *
 * @brief Accepts the set of "safe" behaviors.
 *
 * The automaton description is read from file.
 * The safety automaton is made up of finitely many states, and the transitions between the states are labeled with disturbance inputs. The transitions can in general be non-deterministic. The acceptance condition used is universal acceptance.
 * The state with index 0 is assumed to be the "reject" state
**/
class SafetyAutomaton {
public:
    /** @brief number of component states N **/
    abs_type no_states_;
    /** @brief number of internal disturbance inputs P **/
    abs_type no_inputs_;
    /** @brief post vector: post[(i-1)*P+j] contains the list of posts for state i and dist_input j **/
    std::vector<abs_type>** post_;
public:
    /* copy constructor */
    SafetyAutomaton(const SafetyAutomaton& other) {
        no_states_=other.no_states_;
        no_inputs_=other.no_inputs_;
        post_ = new std::vector<abs_type>*[no_states_*no_inputs_];
        for (int i=0; i<no_states_*no_inputs_; i++) {
            post_[i]=other.post_[i];
        }
    }
    /* constructor */
    SafetyAutomaton() {
        no_states_=0;
        no_inputs_=0;
    }
    /* read description of states and transitions from files */
    void readFromFile(const string& filename) {
        int result = readMember<abs_type>(filename, no_states_, "NO_STATES");
        result = readMember<abs_type>(filename, no_inputs_, "NO_INPUTS");
        abs_type no_elems = no_states_*no_inputs_;
        post_ = new std::vector<abs_type>*[no_elems];
        for (size_t i=0; i<no_elems; i++) {
            std::vector<abs_type> *v=new std::vector<abs_type>;
            post_[i]=v;
        }
        result = readArrVec<abs_type>(filename, post_, no_elems, "TRANSITION_POST");
        for (size_t i=0; i<no_elems; i++) {
            for (size_t j=0; j<post_[i]->size(); j++) {
                if ((*post_[i])[j]>=no_states_) {
                    try {
                        throw std::runtime_error("SafetAutomaton: One of the post state indices is out of bound.");
                    } catch (std::exception& e) {
                        std::cout << e.what() << "\n";
                    }
                }
            }
        }
    }
    /* Destructor */
    ~SafetyAutomaton() {
        delete[] post_;
    }
    /* reset post */
    void resetPost() {
        delete[] post_;
    }
    /* add post */
    void addPost(std::vector<abs_type>** post) {
        /* now set the new post as the one supplied */
        int no_elems = no_states_*no_inputs_;
        post_ = new std::vector<abs_type>*[no_elems];
        for (int i=0; i<no_elems; i++) {
            std::vector<abs_type>* v=new std::vector<abs_type>;
            post_[i]=v;
            for (int j=0; j<post[i]->size(); j++) {
                post_[i]->push_back((*post[i])[j]);
            }
        }
    }
    /*! Address of post in post array.
     * \param[in] i           state index
     * \param[in] j           control input index
     * \param[out] ind    address of the post state vector in post **/
    inline int addr(const abs_type i, const abs_type j) {
        return (i*no_inputs_ + j);
    }
};/* end of class defintion */
}/* end of namespace negotiation */
#endif
