/* Negotiate.hpp
 *
 *  Create by: Kaushik
 *  Date: 10/01/2020    */

/** @file **/
#ifndef NEGOTIATE_HPP_
#define NEGOTIATE_HPP_

#include <vector>
#include <unordered_set>
#include <bits/stdc++.h> /* for setting max_depth_=highest possible integer by default */

#include "Component.hpp" /* for the definition of data types abs_type and abs_ptr_type */
//#include "FileHandler.hpp"
#include "LivenessGame.hpp"

/** @namespace negotiation **/
namespace negotiation {

/**
 * @class Negotiate
 *
 * @brief A wrapper class for the negotiation process.
 */
class Negotiate {
public:
    /** @brief components **/
    std::vector<negotiation::Component*> components_;
    /** @brief local safety specifications of the components **/
    std::vector<std::unordered_set<negotiation::abs_type>*> safe_states_;
    /** @brief local liveness specifications of the components **/
    std::vector<std::unordered_set<negotiation::abs_type>*> target_states_;
    /** @brief set of local guarantees for each component (same as the assumption of the other component) **/
    std::vector<negotiation::SafetyAutomaton*> guarantee_;
    /** @brief maximum  search depth **/
    const int max_depth_;
public:
    /* constructor */
    Negotiate(const std::vector<std::string*> component_files,
              const std::vector<std::string*> safe_states_files,
              const std::vector<std::string*> target_states_files,
              const int max_depth=INT_MAX) : max_depth_(max_depth) {
        /* sanity check */
        if (component_files.size()!=safe_states_files.size() ||
            component_files.size()!=target_states_files.size() ||
            safe_states_files.size()!=target_states_files.size()) {
            try {
                throw std::runtime_error("Negotiate: the number of components, safety specification, and liveness specificaitons supplied do not mathc.");
            } catch (std::exception& e) {
                std::cout << e.what() << "\n";
            }
        }
        /* initialize the components */
        for (int i=0; i<component_files.size(); i++) {
            negotiation::Component* c = new negotiation::Component(*component_files[i]);
            components_.push_back(c);
        }
        /* initialize the sets of safe states for each component */
        for (int i=0; i<safe_states_files.size(); i++) {
            size_t n_safe_states;
            readMember(*safe_states_files[i], n_safe_states, "NO_SAFE_STATES");
            std::unordered_set<abs_type>* s = new std::unordered_set<abs_type>;
            readSet(*safe_states_files[i], *s, n_safe_states, "SET_SAFE_STATES");
            safe_states_.push_back(s);
        }
        /* intialize the sets of target states (for liveness specifications) for each component */
        for (int i=0; i<target_states_files.size(); i++) {
            size_t n_target_states;
            readMember(*target_states_files[i], n_target_states, "NO_TARGET_STATES");
            std::unordered_set<abs_type>* t = new std::unordered_set<abs_type>;
            readSet(*target_states_files[i], *t, n_target_states, "SET_TARGET_STATES");
            target_states_.push_back(t);
        }
        /* initialize the sets of guarantees as all accepting safety automata */
        for (int c=0; c<2; c++) {
            negotiation::SafetyAutomaton* s=new negotiation::SafetyAutomaton(components_[c]->no_outputs);
            guarantee_.push_back(s);
        }
    }
    /*! Perform a negotiation by progrssively increasing the length of spoiling behaviors. */
    bool iterative_deepening_search() {
        /* initialize the length of spoiling behavior set */
        int k=0;
        /* actual k used for the length of the spoiling behavior set */
        int k_now[2]={-1,-1};
        int k_old[2]={-1,-1};
        /* the flag which is true if a solution is reached */
        bool success;
        /* index of the component starting negotiation: inconsequential to the outcome */
        int starting_component=0;
        /* for warm-starting the negotiation process by the initially computed spoilers for the starting component */
        negotiation::SafetyAutomaton* s_init = new negotiation::SafetyAutomaton();
        std::cout << "\n\nInitiating pre-computation of spoilers for the starting component " << starting_component << "\n";
        int init_winning = compute_spoilers_overall(starting_component,s_init);
        /* if the game is losing for the starting component, then change the starting component and try again */
        if (init_winning==0) {
            std::cout << "\tThe game is sure losing for component " << starting_component << ".\n";
            /* flip the starting component and compute the spoiling behavior */
            starting_component=1-starting_component;
            std::cout << "Initiating pre-computation of spoilers for the new starting component " << starting_component << "\n";
            std::cout << "\tComputing spoiler for component " << starting_component << ".\n";
            init_winning=compute_spoilers_overall(starting_component,s_init);
            if(init_winning==0) {
                std::cout << "The game is sure losing for both components. Negotiation is not possible. Terminating.\n";
                return false;
            }
        }
        /* negotiate until either a solution is found, or until increasing k does not change anything */
        while (1) {
            /* stop when maximum depth is reached */
            if (k==max_depth_) {
                std::cout << "Maximum search depth of spoiling behavior reached. No solution found. Terminating." << '\n';
            }
            /* recursively perform the negotiation */
            std::cout << "current depth = " << k << std::endl;
            /* flag for checking success */
            bool success;
            if (init_winning==2) {
                /* the initial component is sure winning, so start with the other component, and noting that one of the components is winning */
                std::cout << "\tThe game is sure winning for component " << starting_component << "." << '\n';
                success = recursive_negotiation(k,k_now,1-starting_component,1);
            } else {
                /* find the spoilers for the starting_component upto the current depth, and update the current set of assumptions and guarantees */
                std::cout << "\tCompressing spoilers for component " << starting_component << "." << '\n';
                /* object for minimizing the spoiling behaviors */
                negotiation::Spoilers spoiler(s_init);
                /* minimize spoilers */
                spoiler.boundedBisim(k);
                /* update the actual depth of minimization achieved */
                k_now[starting_component]=spoiler.k_;
                /* determinize the minimized spoiler automaton */
                spoiler.spoilers_mini_->determinize();
                /* minimize the guarantee automaton further before saving */
                negotiation::Spoilers guarantee_final(spoiler.spoilers_mini_);
                guarantee_final.boundedBisim();
                /* update the guarantee required from the other component */
                *guarantee_[1-starting_component]=*guarantee_final.spoilers_mini_;
                 /* debug */
                guarantee_[0]->writeToFile("Outputs/guarantee_0.txt");
                guarantee_[1]->writeToFile("Outputs/guarantee_1.txt");
                /* debug end */
                success = recursive_negotiation(k,k_now,1-starting_component,0);
            }
            if (success) {
                return true;
            } else {
                /* if there was no change in the depth of bounded bisimulation in the first round of spoiler finding, then stop the procedure */
                if (k_now[0]==k_old[0] && k_now[1]==k_old[1]) {
                    std::cout << "The search of spoiling behavior got saturated. No solution found. Terminating." << '\n';
                    return false;
                } else {
                    k_old[0]=k_now[0];
                    k_old[1]=k_now[1];
                    k_now[0]=-1;
                    k_now[1]=-1;
                }
                /* reset the sets of guarantees */
                guarantee_.clear();
                /* re-initialize the sets of guarantees as all accepting safety automata */
                for (int c=0; c<2; c++) {
                    negotiation::SafetyAutomaton* s=new negotiation::SafetyAutomaton(components_[c]->no_outputs);
                    guarantee_.push_back(s);
                }
                /* increment k and continue the search */
                k++;
            }
        }
    }
    /*! Perform negotiation recursively with fixed length of spoiling behavior.
     * \param[in] k         prescribed length of the spoiling behavior set.
     * \param[in] k_act     actual length of the spoiling behavior used in the first round of finding the spoilers for both components (could be smaller than k_max due to saturation of the bounded bisimulation algorithm).
     * \param[in] c         the index of the component who gets to compute the spoiling behaviors in this round.
     * \param[in] done  a counter counting the number of components which can surely win with the current contracts.
     * \param[out] true/false   success/failure of the negotiation. */
    bool recursive_negotiation(const int k, int* k_act, const int c, int done) {
        std::cout << "\tTurn = " << c << '\n';
        // /* debugging: save interim results */
        // checkMakeDir("Outputs/InterimSets");
        // std::string Str = "";
        // Str += "Outputs/InterimSets/G";
        // Str += std::to_string(c);
        // Str += ".txt";
        // char Char[20];
        // size_t Length = Str.copy(Char, Str.length() + 1);
        // Char[Length] = '\0';
        // guarantee_[c]->writeToFile(Char);
        // /* end of debugging */
        negotiation::SafetyAutomaton* s = new negotiation::SafetyAutomaton();
        std::cout << "\tComputing spoiler for component " << c << ".\n";
        int flag = compute_spoilers_overall(c,s);
       //  /* debug */
       // s->writeToFile("Outputs/spoiler.txt");
       //  /* debug ends */
        if (flag==0) {
            /* when the game is sure losing for component c, the negotiation fails */
            std::cout << "\tThe game is sure losing for component " << c << "." << '\n';
            return false;
        } else if (done==1 && flag==2) {
            /* when both the components have sure winning strategies with the current set of contracts, the negotiation successfully terminates */
            std::cout << "\tThe game is sure winning for both of the components. The negoitaiotn succeeded. Terminating the process." << '\n';
            return true;
        } else if (done==0 && flag==2) {
            /* when the component c---but not (1-c)---has a sure winning strategy with the present contract, it's component (1-c)'s turn to compute the spoilers */
            std::cout << "\tThe game is sure winning for component " << c << "." << '\n';
            return recursive_negotiation(k,k_act,1-c,done+1);
        } else {
            /* compress the spoilers for component c, and update the current set of assumptions and guarantees */
            std::cout << "\tCompressing spoilers for component " << c << "." << '\n';
            negotiation::Spoilers spoiler(s);
            spoiler.boundedBisim(k);
            if (k_act[c]==-1) {
                k_act[c]=spoiler.k_;
            }
             // /* debug */
             // spoiler.spoilers_mini_->writeToFile("Outputs/spoiler_mini.txt");
             // /* debug ends */
            negotiation::SafetyAutomaton guarantee_updated(*guarantee_[1-c],*spoiler.spoilers_mini_);
            guarantee_updated.trim();
            // /*debug */
            // guarantee_updated.writeToFile("Outputs/guarantee_before_det.txt");
            // /*debug ends */
            guarantee_updated.determinize();
            // /*debug */
            // guarantee_updated.writeToFile("Outputs/guarantee_after_det.txt");
            // /*debug ends */

            /* new: minimize the guarantee automaton before saving */
            negotiation::Spoilers guarantee_final(&guarantee_updated);
            guarantee_final.boundedBisim();

            *guarantee_[1-c]=*guarantee_final.spoilers_mini_;
            // /* save the updated guarantee */
            // Str += "Outputs/G";
            // Str += std::to_string(1-c);
            // Str += ".txt";
            // char Char[20];
            // size_t Length = Str.copy(Char, Str.length() + 1);
            // Char[Length] = '\0';
            // guarantee_[1-c]->writeToFile(Char);
             /* debug */
            guarantee_updated.writeToFile("Outputs/full_guarantee.txt");
            guarantee_[0]->writeToFile("Outputs/guarantee_0.txt");
            guarantee_[1]->writeToFile("Outputs/guarantee_1.txt");
             /* debug ends */
            bool flag2 = recursive_negotiation(k,k_act,1-c,0);
            if (flag2) {
                return true;
            } else {
                return false;
            }
        }
    }
    /*! Find the overall spoiling behavior for a given component in the form of a safety automaton.
     * \param[in] c             the component index
     * \param[in] spoilers      the pointer to the safety automaton storing the spoiling behaviors
     * \param[out] out_flag   0 -> some initial states are sure losing, 2 -> all initial states are sure winning, 1-> otherwise. */
    int compute_spoilers_overall(const int c, negotiation::SafetyAutomaton* spoilers) {
        /* the output flag */
        int out_flag;
        /* find the spoilers for the safety part */
        negotiation::SafetyGame monitor(*components_[c],*guarantee_[1-c],*guarantee_[c]);
        std::vector<std::unordered_set<abs_type>*> sure_safe = monitor.solve_safety_game(*safe_states_[c],"sure");
        std::vector<std::unordered_set<abs_type>*> maybe_safe = monitor.solve_safety_game(*safe_states_[c],"maybe");
        SafetyAutomaton* spoilers_safety = new SafetyAutomaton;
        int flag1 = monitor.find_spoilers(sure_safe, maybe_safe, spoilers_safety);
        /* if some initial states are surely losing the safety specification, then return out_flag=0 */
        if (flag1==0) {
            out_flag=0;
            return out_flag;
        }
        spoilers_safety->trim();
        /* minimize the spoiler_safety automaton  */
        negotiation::Spoilers safety(spoilers_safety);
        safety.boundedBisim();
        /* find the spoilers for the liveness part (with the strategies being already restricted by the strategy obtained during the synthesis of the maybe safety controller) */
        SafetyAutomaton* spoilers_liveness = new SafetyAutomaton;
        /* assume that the liveness game is winning */
        int flag2=2;
        // /* if the set of target states is all the states, then skip the liveness part (the liveness game is trivially winning in that case) */
        // if (target_states_[c]->size()!=components_[c]->no_states) {
            std::vector<std::unordered_set<abs_type>*> allowed_joint_inputs;
            if (flag1==2) {
                /* if the safety game was sure winning, then the only restriction on input choices during the liveness game part comes from the sure winning strategy */
                for (abs_type i=0; i<monitor.no_states; i++) {
                    std::unordered_set<abs_type>* s = new std::unordered_set<abs_type>;
                    for (auto l=sure_safe[i]->begin(); l!=sure_safe[i]->end(); ++l) {
                        for (abs_type k=0; k<monitor.no_dist_inputs; k++) {
                            s->insert(monitor.addr_uw(*l,k));
                        }
                    }
                    allowed_joint_inputs.push_back(s);
                }
            } else {
                /* otherwise, the restriction on joint inputs come from the maybe winning strategy (the spoiler automaton comupted from the safety part will take care of the fact that the correct disturbance inputs are available at the correct point) */
                for (abs_type i=0; i<monitor.no_states; i++) {
                    std::unordered_set<abs_type>* s = new std::unordered_set<abs_type>;
                    *s=*maybe_safe[i];
                    allowed_joint_inputs.push_back(s);
                }
            }
            negotiation::LivenessGame monitor_live(monitor, *target_states_[c], sure_safe, allowed_joint_inputs);
            flag2 = monitor_live.find_spoilers(spoilers_liveness);
            /* if some initial states are surely losing the liveness specification, then return false */
            if (flag2==0) {
                out_flag=0;
                return out_flag;
            }
            spoilers_liveness->trim();
        // }
        /* minimize the spoiler_safety automaton  */
        negotiation::Spoilers liveness(spoilers_liveness);
        liveness.boundedBisim();
        /* the overall spoiling behavior is the union of spoiling behavior for the safety spec and the liveness spec, or the overall non-spoiling behavior is the intersection of non-spoilers for safety AND non-spoilers for liveness */
        SafetyAutomaton spoilers_overall(*safety.spoilers_mini_, *liveness.spoilers_mini_);
        spoilers_overall.trim();
        /* minimize the spoiler_overall automaton  */
        negotiation::Spoilers overall(&spoilers_overall);
        overall.boundedBisim();
        /* copy the overall spoiling behavior to the one supplied as input for storing the spoiling behaviors */
        *spoilers=*overall.spoilers_mini_;
        /* if both the safety and the liveness games are sure winning, then return out_flag=2, else return out_flag=1 */
        if (flag1==2 && flag2==2) {
            out_flag=2;
        } else {
            out_flag=1;
        }
        return out_flag;
    }
    /* Perform the bisimulation algorithm implemented for spoiler minimization in a forward direction.
     *      This algorithm is based on the bounded bisimulation in spoiler class.
            - First a spoiler object is created by inverting all the arrows in the safety automaton.
            - Then the intitial states are lumped into one initial state, and the indices of the initial state and the sink state 0 are flipped.
            - After that, the bounded bisimulation algorithm is run until convergence, and
            - in the end all the arrows and the intial state and the sink state are restored to their normal form to obtain the final deterministic automaton.*/
    void reverse_bisimulation(SafetyAutomaton* A) {
        /* first create a new safety automaton whose transitions are the inverted version of this safety automaton */
        negotiation::SafetyAutomaton safety_altered;
        /* the number of states of safety_altered is obtained by considering one initial state instead of many */
        safety_altered.no_states_=A->no_states_-A->init_.size()+1;
        /* number of inputs is the same */
        safety_altered.no_inputs_=A->no_inputs_;
        /* all the initial states are grouped together and mapped to 0, while the rejecting sink state is mapped to the smallest initial state index */
        safety_altered.init_.insert(smallest_element<abs_type>(A->init_));
        /* create the transition array */
        std::unordered_set<abs_type>** post = new std::unordered_set<abs_type>*[safety_altered.no_states_*safety_altered.no_inputs_];
        for (abs_type i=0; i<safety_altered.no_states_; i++) {
            for (abs_type k=0; k<safety_altered.no_inputs_; k++) {
                std::unordered_set<abs_type>* set = new std::unordered_set<abs_type>;
                post[safety_altered.addr(i,k)]=set;
            }
        }
        /* the transitions are inverted */
        for (abs_type i=0; i<A->no_states_; i++) {
            for (abs_type k=0; k<A->no_inputs_; k++) {
                for (auto i2=A->post_[A->addr(i,k)]->begin(); i2!=A->post_[A->addr(i,k)]->end(); ++i2) {
                    abs_type cur_state, next_state;
                    if (i==0) {
                        next_state=*safety_altered.init_.begin(); /* there is just one state in the new init_ */
                    } else if (A->init_.find(i)!=A->init_.end()) {
                        next_state=0;
                    } else {
                        next_state=i;
                    }
                    if (*i2==0) {
                        cur_state=*safety_altered.init_.begin(); /* there is just one state in the new init_ */
                    } else if (A->init_.find(*i2)!=A->init_.end()) {
                        cur_state=0;
                    } else {
                        cur_state=*i2;
                    }
                    post[safety_altered.addr(cur_state,k)]->insert(next_state);
                }
            }
        }
        safety_altered.addPost(post);
        // /* debug */
        // safety_altered.writeToFile("Outputs/interim_safety_altered.txt");
        // /* debug end */
        /* perform bisimulation (implemented in the spoiler class) */
        Spoilers spoiler_dummy(&safety_altered);
        spoiler_dummy.boundedBisim();
        /* the new number of states is obtained by grouping together all the initial states of the minimized automaton into one state */
        A->no_states_=spoiler_dummy.spoilers_mini_->no_states_-spoiler_dummy.spoilers_mini_->init_.size()+1;
        /* the new initial state (singleton) is mapped to the smallest initial state index of the minimized spoiler automaton */
        A->init_.clear();
        A->init_.insert(smallest_element<abs_type>(spoiler_dummy.spoilers_mini_->init_));
        /* invert the transitions and the initial and sink states in the minimized automaton again */
        std::unordered_set<abs_type>** post_new = new std::unordered_set<abs_type>*[A->no_states_*A->no_inputs_];
        /* first fill the post_new array */
        for (abs_type i=0; i<A->no_states_; i++) {
            for (abs_type k=0; k<A->no_inputs_; k++) {
                std::unordered_set<abs_type>* set = new std::unordered_set<abs_type>;
                post_new[A->addr(i,k)]=set;
            }
        }
        /* invert the transitions */
        for (abs_type i=0; i<spoiler_dummy.spoilers_mini_->no_states_; i++) {
            for (abs_type k=0; k<spoiler_dummy.spoilers_mini_->no_inputs_; k++) {
                for (auto i2=spoiler_dummy.spoilers_mini_->post_[spoiler_dummy.spoilers_mini_->addr(i,k)]->begin(); i2!=spoiler_dummy.spoilers_mini_->post_[spoiler_dummy.spoilers_mini_->addr(i,k)]->end(); ++i2) {
                    abs_type cur_state, next_state;
                    if (i==0) {
                        next_state=*A->init_.begin(); /* there is just one state in the new init_ */
                    } else if (spoiler_dummy.spoilers_mini_->init_.find(i)!=spoiler_dummy.spoilers_mini_->init_.end()) {
                        next_state=0;
                    } else {
                        next_state=i;
                    }
                    if (*i2==0) {
                        cur_state=*A->init_.begin(); /* there is just one state in the new init_ */
                    } else if (spoiler_dummy.spoilers_mini_->init_.find(*i2)!=spoiler_dummy.spoilers_mini_->init_.end()) {
                        cur_state=0;
                    } else {
                        cur_state=*i2;
                    }
                    post_new[A->addr(cur_state,k)]->insert(next_state);
                }
            }
        }
        A->resetPost();
        A->addPost(post_new);

        delete[] post;
        delete[] post_new;
    }
private:
    /* find the smallest element in a given set */
    template <class T>
    T smallest_element(const std::unordered_set<T>& set) {
        T elem = *set.begin();
        for (auto i=set.begin(); i!=set.end(); ++i) {
            if (elem>*i) {
                elem=*i;
            }
        }
        return elem;
    }
}; /* end of class definition */
} /* end of namespace negotiation */
#endif
