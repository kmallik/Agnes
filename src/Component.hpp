/* Component.hpp
 *
 *  Created by: Kaushik
 *  Date: 12/11/2019    */

/** @file **/
#ifndef COMPONENT_HH_
#define COMPONENT_HH_

#include <iostream>
#include <vector>
#include <string>

//#include "Functions.hpp"
#include "FileHandler.hpp"

/** @namespace negotiation **/
namespace negotiation {

/**
 * @brief abs_type defines the type used to denote the state indices
 * implicitly determines an upper bound on the number of states (2^32-1)
 */
using abs_type=std::uint32_t;

/**
 * @brief abs_ptr_type defines type used to point to the array m_pre (default = uint64_t) \n
 * determinse implicitely an upper bound on the number of transitions (default = * 2^64-1)
 **/
using abs_ptr_type=std::uint64_t;

using namespace std;

/**
 *  @class Component
 *
 *  @brief The finite transition system modeling the components
 *
 *  The Component description is read from file.
 *  The Component is made up of finitely many states, and the transitions between the states are labeled with control and (internal) disturbance inputs. The transitions can in general be non-deterministic, where the non-determinism models the effect of external (possibly adversarial) environment.
 **/
class Component {
public:
    /** @brief number of component states N **/
    abs_type no_states;
    /** @brief number of control inputs M **/
    abs_type no_control_inputs;
    /** @brief number of internal disturbance inputs P **/
    abs_type no_dist_inputs;
    /** @brief number of outputs R  ( smaller than N) **/
    abs_type no_outputs;
    /** @brief vector[N] containing the output mapping **/
    std::vector<abs_type> state_to_output;
    /** @brief vector[R] containing the state indices **/
    std::vector<abs_type> output_to_state;
    /** @brief post array: the index ( i*M*P + j*P + k ) holds the list of post states for the (state,control,disturbance,post_state)  tuple (i,j,k), where i, j, k start from 0,..**/
    std::vector<abs_type>** post;
public:
    /* copy constructor */
    Component(const Component& other) {
        no_states=other.no_states;
        no_control_inputs=other.no_control_inputs;
        no_dist_inputs=other.no_dist_inputs;
        no_outputs=other.no_outputs;
        state_to_output=other.state_to_output;
        output_to_state=other.output_to_state;
        abs_type size = no_states*no_control_inputs*no_dist_inputs;
        post = new std::vector<abs_type>*[size];
        for (int i=0; i<size; i++) {
            post[i]=other.post[i];
        }
    }
    /* Destructor */
    ~Component() {
        abs_type no_elems = no_states*no_control_inputs*no_dist_inputs;
        for (int i=0; i<no_elems; i++) {
            delete post[i];
        }
    }
    /* @endcond */
    /* constructor */
    Component(const string& filename) {
        int result = readMember<abs_type>(filename, no_states, "NO_STATES");
        result = readMember<abs_type>(filename, no_control_inputs, "NO_CONTROL_INPUTS");
        result = readMember<abs_type>(filename, no_dist_inputs, "NO_DIST_INPUTS");
        result = readMember<abs_type>(filename, no_outputs, "NO_OUTPUTS");
        state_to_output.clear();
        result = readVec<abs_type>(filename, state_to_output, no_states, "STATE_TO_OUTPUT");
        for (abs_type i=0; i<no_states; i++) {
            if (state_to_output[i]>=no_outputs) {
                try {
                    throw std::runtime_error("Component: output index out of bound.");
                } catch (std::exception& e) {
                    std::cout << e.what() << "\n";
                }
            }
        }
        output_to_state.clear();
        std::vector<abs_type> output_to_state(no_outputs);
        for (size_t i=0; i<no_states; i++) {
            output_to_state[state_to_output[i]]=i;
        }
        abs_type no_post_elems = no_states*no_control_inputs*no_dist_inputs;
        post = new std::vector<abs_type>*[no_post_elems];
        for (size_t i=0; i<no_post_elems; i++) {
            std::vector<abs_type> *v = new std::vector<abs_type>;
            post[i]=v;
        }
        result = readArrVec<abs_type>(filename, post, no_post_elems, "TRANSITION_POST");
    }
    /*! Address of post in post array.
     * \param[in] i           state index
     * \param[in] j           control input index
     * \param[in] k           disturbance input index
     * \param[out] ind    address of the post state vector in post **/
    inline int addr(const abs_type i, const abs_type j, const abs_type k) {
        return (i*no_control_inputs*no_dist_inputs + j*no_dist_inputs + k);
    }
};

} /* end of namespace negotiation */
#endif /* COMPONENT_HH_ */
