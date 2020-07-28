/* Component.hpp
 *
 *  Created by: Kaushik
 *  Date: 12/11/2019    */

/** @file **/
#ifndef COMPONENT_HPP_
#define COMPONENT_HPP_

#include <iostream>
#include <vector>
#include <string>

#include "FileHandler.hpp"
#include "DotInterface.hpp"

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
    /** @brief set of initial state indices **/
    std::unordered_set<abs_type> init_;
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
    /*! Copy constructor */
    Component(const Component& other) {
        no_states=other.no_states;
        init_=other.init_;
        no_control_inputs=other.no_control_inputs;
        no_dist_inputs=other.no_dist_inputs;
        no_outputs=other.no_outputs;
        state_to_output=other.state_to_output;
        output_to_state=other.output_to_state;
        abs_type size = no_states*no_control_inputs*no_dist_inputs;
        post = new std::vector<abs_type>*[size];
        for (abs_type i=0; i<size; i++) {
            post[i]=other.post[i];
        }
    }
    /*! The destructor */
    ~Component() {
        abs_type no_elems = no_states*no_control_inputs*no_dist_inputs;
        for (abs_type i=0; i<no_elems; i++) {
            delete post[i];
        }
    }
    /* @endcond */
    /*!
     *  The constructor takes as input the name of the file that contains the encoding of the component members.
     */
    Component(const string& filename) {
        int result = readMember<abs_type>(filename, no_states, "NO_STATES");
        abs_type ni;
        result = readMember<abs_type>(filename, ni, "NO_INITIAL_STATES");
        result = readSet<abs_type>(filename, init_, ni, "INITIAL_STATE_LIST");
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
    /*! Index of the control input from a joint control-disturbance index.
     * \param[in] l             joint control-disturbance input index
     * \param[out] j           control input index */
    inline abs_type cont_ind(const abs_type l) {
        return (l / no_dist_inputs);
    }
    /*! Save the description of the safety automaton as a directed graph with clusterd vertices
     * \param[in] filename      the output file name
     * \param[in] graph_name  the name of the graph
     * \param[in] state_labels  the labels to the state which will appear in the visualization (the size must be same as the number of states)
     * \param[in] state_clusters  the groups of states which should appear close to each other (possibly for better visualization)
     * \param[in] control_input_labels the labels to the control inputs which will appear in the visualization (the size must be same as the number of control inputs)
     * \param[in] dist_input_labels the labels to the disturbance inputs which will appear in the visualization (the size must be same as the number of disturbance inputs) */
    void createDOT(const std::string& filename,
                    const std::string& graph_name,
                    const std::vector<std::string*> state_labels,
                    const std::vector<std::unordered_set<abs_type>*> state_clusters,
                    const std::vector<std::string*> control_input_labels,
                    const std::vector<std::string*> dist_input_labels) {
        /* the number of elements in state_labels needs to match with the number of states */
        if (state_labels.size()!=no_states) {
            throw std::runtime_error("Component:createDOT: Number of state labels does not match with the number of states.");
        }
        /* the number of elements in control_input_labels needs to match with the number of inputs */
        if (control_input_labels.size()!=no_control_inputs) {
            throw std::runtime_error("Component:createDOT: Number of control input labels does not match with the number of control inputs.");
        }
        /* the number of elements in dist_input_labels needs to match with the number of inputs */
        if (dist_input_labels.size()!=no_dist_inputs) {
            throw std::runtime_error("Component:createDOT: Number of disturbance input labels does not match with the number of disturbance inputs.");
        }
        /* hyphen is a reserved label for universal choice for both control or joint inputs */
        for (size_t i=0; i<control_input_labels.size(); i++) {
            if (*control_input_labels[i]=="-") {
                throw std::runtime_error("Component:createDOT: hyphen cannot be used as a label for control inputs.");
            }
        }
        for (size_t i=0; i<dist_input_labels.size(); i++) {
            if (*dist_input_labels[i]=="-") {
                throw std::runtime_error("Component:createDOT: hyphen cannot be used as a label for disturbance inputs.");
            }
        }
        /* create labels for joint inputs: add extra labels for cases when all the inputs of one type have the same effect on the transitions */
        std::vector<std::string*> joint_input_labels;
        for (abs_type j=0; j<no_control_inputs+1; j++) {
            for (abs_type k=0; k<no_dist_inputs+1; k++) {
                std::string* s=new std::string;
                *s="";
                if (j==no_control_inputs) {
                    *s+="-";
                } else {
                    *s+=*control_input_labels[j];
                }
                *s+="/";
                if (k==no_dist_inputs) {
                    *s+="-";
                } else {
                    *s+=*dist_input_labels[k];
                }
                joint_input_labels.push_back(s);
            }
        }
        /* the address of the post vector in the "transitions" array is determined using the following lambda expression */
        auto post_addr = [&](abs_type x, abs_type u, abs_type w) -> abs_type {
            return (x*(no_control_inputs+1)*(no_dist_inputs+1) + u*(no_dist_inputs+1) + w);
        };
        /* create a post array */
        std::unordered_set<abs_type>** post_array=new std::unordered_set<abs_type>*[no_states*(no_control_inputs+1)*(no_dist_inputs+1)];
        for (abs_type i=0; i<no_states*(no_control_inputs+1)*(no_dist_inputs+1); i++) {
            std::unordered_set<abs_type>* set=new std::unordered_set<abs_type>;
            post_array[i]=set;
        }
        /* the set of all states in one vector */
        std::vector<abs_type> all_states;
        for (abs_type i=0; i<no_states; i++) {
            all_states.push_back(i);
        }
        for (abs_type i=0; i<no_states; i++) {
            /* keep track of the universal successors */
            std::vector<abs_type> overall_univ_succ;
            for (abs_type j=0; j<no_control_inputs; j++) {
                /* check the universal successors for this control input */
                std::vector<abs_type> univ_succ=all_states;
                for (abs_type k=0; k<no_dist_inputs; k++) {
                    univ_succ=vecIntersect(univ_succ,*post[addr(i,j,k)]);
                }
                /* bookkeeping */
                overall_univ_succ=vecUnion(overall_univ_succ,univ_succ);
                /* the universal successors for this control inputs corresponding to the special disturbance index "no_dist_inputs" */
                for (auto l=univ_succ.begin(); l!=univ_succ.end(); l++) {
                    post_array[post_addr(i,j,no_dist_inputs)]->insert(*l);
                }
            }
            for (abs_type k=0; k<no_dist_inputs; k++) {
                /* check the universal successors for this disturbance input */
                std::vector<abs_type> univ_succ=all_states;
                for (abs_type j=0; j<no_control_inputs; j++) {
                    univ_succ=vecIntersect(univ_succ,*post[addr(i,j,k)]);
                }
                /* if some of the universal successors of this disturbance input is also in the universal successor of some other control input, then move that successor to the joint universal control and universal disturbance successor */
                for (auto l=univ_succ.begin(); l!=univ_succ.end(); ++l) {
                    bool not_univ_for_control=true;
                    for (abs_type j=0; j<no_control_inputs; j++) {
                        if (post_array[post_addr(i,j,no_dist_inputs)]->find(*l) != post_array[post_addr(i,j,no_dist_inputs)]->end()) {
                            not_univ_for_control=false;
                            post_array[post_addr(i,j,no_dist_inputs)]->erase(*l);
                            post_array[post_addr(i,no_control_inputs,no_dist_inputs)]->insert(*l);
                            break;
                        }
                    }
                    if (not_univ_for_control) {
                        /* the rest of the universal successors for this disturbance inputs corresponding to the special control index "no_control_inputs" */
                        for (auto l=univ_succ.begin(); l!=univ_succ.end(); l++) {
                            post_array[post_addr(i,no_control_inputs,k)]->insert(*l);
                        }

                    }
                }
                /* bookkeeping */
                overall_univ_succ=vecUnion(overall_univ_succ,univ_succ);
            }
            /* add the non-universal successors to the list of post */
            for (abs_type j=0; j<no_control_inputs; j++) {
                for (abs_type k=0; k<no_dist_inputs; k++) {
                    for (auto l=post[addr(i,j,k)]->begin(); l!=post[addr(i,j,k)]->end(); l++) {
                        bool not_in_universal=true;
                        for (auto i2=overall_univ_succ.begin(); i2!=overall_univ_succ.end(); ++i2) {
                            if (*l==*i2) {
                                not_in_universal=false;
                                break;
                            }
                        }
                        if (not_in_universal) {
                            post_array[post_addr(i,j,k)]->insert(*l);
                        }
                    }
                }
            }
        }
        /* write the graph */
        createDiGraph<abs_type>(filename, graph_name, state_labels, state_clusters, init_, joint_input_labels, post_array);

        delete[] post_array;
    }
    /*! Save the description of the safety automaton as a simple directed graph  without any clusters of vertices
     * \param[in] filename      the output file name
     * \param[in] graph_name  the name of the graph
     * \param[in] state_labels  the labels to the state which will appear in the visualization (the size must be same as the number of states)
     * \param[in] control_input_labels the labels to the control inputs which will appear in the visualization (the size must be same as the number of control inputs)
     * \param[in] dist_input_labels the labels to the disturbance inputs which will appear in the visualization (the size must be same as the number of disturbance inputs)*/
    void createDOT(const std::string& filename,
                    const std::string& graph_name,
                    const std::vector<std::string*> state_labels,
                    const std::vector<std::string*> control_input_labels,
                    const std::vector<std::string*> dist_input_labels) {
        /* create an empty list of clusters of vertices and call the createDOT function with clusters */
        std::vector<std::unordered_set<abs_type>*> state_clusters;
        createDOT(filename, graph_name, state_labels, state_clusters, control_input_labels, dist_input_labels);
    }
private:
    /*! Compute union of two vectors
     * \param[in] v1  the first vector
     * \param[in] v2  the second vector
     * \param[out] v  the output vector which contains all the elements that appear in either v1 or v2 */
    template<class T>
    std::vector<T> vecUnion(const std::vector<T>& v1, const std::vector<T>& v2) {
        std::vector<T> v=v1;
        for (auto i2=v2.begin(); i2!=v2.end(); ++i2) {
            bool this_element_new=true;
            for (auto i1=v.begin(); i1!=v.end(); ++i1) {
                if (*i1==*i2) {
                    /* the current element was already added */
                    this_element_new=false;
                    break;
                }
            }
            if (this_element_new) {
                v.push_back(*i2);
            }
        }
        return v;
    }
    /*! Compute intersection of two vectors
     * \param[in] v1  the first vector
     * \param[in] v2  the second vector
     * \param[out] v  the output vector which contains all the elements that appear in both v1 or v2*/
    template<class T>
    std::vector<T> vecIntersect(const std::vector<T>& v1, const std::vector<T>& v2) {
        std::vector<T> v;
        for (auto i1=v1.begin(); i1!=v1.end(); ++i1) {
            for (auto i2=v2.begin(); i2!=v2.end(); ++i2) {
                if (*i1==*i2) {
                    /* the current element is in the intersection */
                    v.push_back(*i1);
                    break;
                }
            }
        }
        return v;
    }
    /*! Compute union of two unordered sets
     * \param[in] s1  the first set
     * \param[in] s2  the second set
     * \param[out] s  the output set which contains all the elements that appear in either s1 or s2*/
    template<class T>
    std::vector<T> setUnion(const std::unordered_set<T>& s1, const std::unordered_set<T>& s2) {
        std::vector<T> s=s1;
        for (auto i2=s2.begin(); i2!=s2.end(); ++i2) {
            s.insert(*i2);
        }
        return s;
    }
};

} /* end of namespace negotiation */
#endif /* COMPONENT_HH_ */
