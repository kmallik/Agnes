/* SafetyAutomaton.hpp
 *
 *  Created by: Kaushik
 *  Date: 16/11/2019  */

/** @file **/
#ifndef SAFETYAUTOMATON_HH_
#define SAFETYAUTOMATON_HH_

#include <vector>

#include "FileHandler.hpp"
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
    abs_type no_states;
    /** @brief number of internal disturbance inputs P **/
    abs_type no_dist_inputs;
    /** @brief post vector: post[(i-1)*P+j] contains the list of posts for state i and dist_input j **/
    std::vector<abs_type>** post;
public:
    /* copy constructor */
    SafetyAutomaton(const SafetyAutomaton& other) {
        no_states=other.no_states;
        no_dist_inputs=other.no_dist_inputs;
        post = new std::vector<abs_type>*[no_states*no_dist_inputs];
        for (int i=0; i<no_states*no_dist_inputs; i++) {
            post[i]=other.post[i];
        }
    }
    /* constructor */
    SafetyAutomaton(const string& filename) {
        int result = readMember<abs_type>(filename, no_states, "NO_STATES");
        result = readMember<abs_type>(filename, no_dist_inputs, "NO_DIST_INPUTS");
        result = readVecArr<abs_type>(filename, post, no_states*no_dist_inputs, "TRANSITION_POST");
    }
    /*! Address of post in post array.
     * \param[in] i           state index
     * \param[in] j           control input index
     * \param[out] ind    address of the post state vector in post **/
    inline int addr(const int i, const int j) {
        return ((i-1)*no_dist_inputs + j);
    }
    
};/* end of class defintion */
}/* end of namespace negotiation */
#endif
