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
    /** @brief verbosity level between 0 (not verbose) to 2 (debug level verbose) **/
    const int verbose_;
public:
    /* constructor */
    Negotiate(const std::vector<std::string*> component_files,
              const std::vector<std::string*> safe_states_files,
              const std::vector<std::string*> target_states_files,
              const int max_depth=INT_MAX,
              const int verbose=0) : max_depth_(max_depth), verbose_(verbose) {
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
        for (size_t i=0; i<component_files.size(); i++) {
            negotiation::Component* c = new negotiation::Component(*component_files[i]);
            components_.push_back(c);
        }
        /* initialize the sets of safe states for each component */
        for (size_t i=0; i<safe_states_files.size(); i++) {
            size_t n_safe_states;
            readMember(*safe_states_files[i], n_safe_states, "NO_SAFE_STATES");
            std::unordered_set<abs_type>* s = new std::unordered_set<abs_type>;
            readSet(*safe_states_files[i], *s, n_safe_states, "SET_SAFE_STATES");
            safe_states_.push_back(s);
        }
        /* intialize the sets of target states (for liveness specifications) for each component */
        for (size_t i=0; i<target_states_files.size(); i++) {
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
    /* resets the guarantees */
    void reset(){
        guarantee_.clear();
        for (int c=0; c<2; c++) {
            negotiation::SafetyAutomaton* s=new negotiation::SafetyAutomaton(components_[c]->no_outputs);
            guarantee_.push_back(s);
        }
    }
    /*! Perform a negotiation by progrssively increasing the length of spoiling behaviors.
     *  \param[in] starting_component   the index of the component which starts the negotation process (default is 0)
     *  \param[out] k   the output flag: k=-1 no contract exists, 0<= k <= k_max negotiation successful, k > k_max negotitation was inconclusive (no contract found, but contract might exist for higher value of k_max). */
    int iterative_deepening_search(int starting_component=0) {
        /* first clear the existing guarantees if any */
        reset();
        /* initialize the length of spoiling behavior set */
        int k=0;
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
                return k;
            }
        }
        /* save debug info */
        if (verbose_>1) {
            s_init->writeToFile("Outputs/spoiler.txt");
        }
        /* variable for checking saturation of the k-minimization */
        bool saturated=false;
        /* negotiate until either a solution is found, or until k reaches maximum depth */
        while (1) {
            /* stop when maximum depth is reached */
            if (k>max_depth_) {
                std::cout << "Maximum search depth of spoiling behavior reached. Existence of solution unknown. Terminating." << '\n';
                return k;
            }
            /* stop when the k-minimization gets saturated */
            if (saturated) {
                std::cout << "Search got saturated in the depth of spoiling behavior. No solution exists. Terminating." << '\n';
                return -1;
            } else {
                saturated=true;
            }
            /* recursively perform the negotiation */
            std::cout << "current depth = " << k << std::endl;
            /* flag for checking success */
            bool success;
            if (init_winning==2) {
                /* the initial component is sure winning, so start with the other component, and noting that one of the components is winning */
                std::cout << "\tThe game is sure winning for component " << starting_component << "." << '\n';
                success = recursive_negotiation(k,1-starting_component,1,saturated);
            } else {
                /* find the spoilers for the starting_component upto the current depth, and update the current set of assumptions and guarantees */
                std::cout << "\tCompressing spoilers for component " << starting_component << "." << '\n';
                /* object for minimizing the spoiling behaviors */
                negotiation::Spoilers spoiler(s_init);
                /* minimize spoilers */
                spoiler.boundedBisim(k);
                /* check saturation */
                if (spoiler.k_==k && saturated) {
                    saturated=false;
                }
                /* determinize the minimized spoiler automaton */
                spoiler.spoilers_mini_->determinize();
                /* minimize the guarantee automaton further before saving */
                negotiation::Spoilers guarantee_final(spoiler.spoilers_mini_);
                guarantee_final.boundedBisim();
                /* update the guarantee required from the other component */
                *guarantee_[1-starting_component]=*guarantee_final.spoilers_mini_;
                /* save debug info */
                if (verbose_>1) {
                    guarantee_[0]->writeToFile("Outputs/guarantee_0.txt");
                    guarantee_[1]->writeToFile("Outputs/guarantee_1.txt");
                }
                success = recursive_negotiation(k,1-starting_component,0,saturated);
            }
            if (success) {
                return k;
            } else {
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
    /*! Perform a negotiation by progrssively increasing the length of spoiling behaviors.
     *  \param[in] k   the depth reached. When k>max_depth_, then this indicates that the negotiation has failed.
     *  \param[in] starting_component   the index of the component which starts the negotation process (default is 0)
     *  \param[out] output_flag     0- no contract exists, 1- negotiation inconclusive (contract might exist for higher value of k), 2- negotiation successful. */
    int fixed_depth_search(int k, int starting_component=0) {
        /* first clear the existing guarantees if any */
        reset();
        /* output flag */
        int output_flag;
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
                return k;
            }
        }
        /* save debug info */
        if (verbose_>1) {
            s_init->writeToFile("Outputs/spoiler.txt");
        }
        /* variable for checking saturation of the k-minimization */
        bool saturated=false;
        /* flag for checking success */
        bool success;
        if (init_winning==2) {
            /* the initial component is sure winning, so start with the other component, and noting that one of the components is winning */
            std::cout << "\tThe game is sure winning for component " << starting_component << "." << '\n';
            success = recursive_negotiation(k,1-starting_component,1,saturated);
        } else {
            /* find the spoilers for the starting_component upto the current depth, and update the current set of assumptions and guarantees */
            std::cout << "\tCompressing spoilers for component " << starting_component << "." << '\n';
            /* object for minimizing the spoiling behaviors */
            negotiation::Spoilers spoiler(s_init);
            /* minimize spoilers */
            spoiler.boundedBisim(k);
            /* check saturation */
            if (spoiler.k_==k && saturated) {
                saturated=false;
            }
            /* determinize the minimized spoiler automaton */
            spoiler.spoilers_mini_->determinize();
            /* minimize the guarantee automaton further before saving */
            negotiation::Spoilers guarantee_final(spoiler.spoilers_mini_);
            guarantee_final.boundedBisim();
            /* update the guarantee required from the other component */
            *guarantee_[1-starting_component]=*guarantee_final.spoilers_mini_;
            /* save debug info */
            if (verbose_>1) {
                guarantee_[0]->writeToFile("Outputs/guarantee_0.txt");
                guarantee_[1]->writeToFile("Outputs/guarantee_1.txt");
            }
            success = recursive_negotiation(k,1-starting_component,0,saturated);
        }
        if (success) {
            output_flag=2;
        } else if (saturated) {
            output_flag=0;
        } else {
            output_flag=1;
        }
        return output_flag;
    }
    /*! Perform negotiation recursively with fixed length of spoiling behavior.
     * \param[in] k         prescribed length of the spoiling behavior set.
     * \param[in] c         the index of the component who gets to compute the spoiling behaviors in this round.
     * \param[in] done  a counter counting the number of components which can surely win with the current contracts.
     * \param[in] is_saturated  a boolean flag checking whether the negotiation process got saturated in the depth of the spoiler minimization
     * \param[out] true/false   success/failure of the negotiation. */
    bool recursive_negotiation(const int k, const int c, int done, bool& is_saturated) {
        std::cout << "\tTurn = " << c << '\n';
        negotiation::SafetyAutomaton* s = new negotiation::SafetyAutomaton();
        std::cout << "\tComputing spoiler for component " << c << ".\n";
        int flag = compute_spoilers_overall(c,s);
        /* save debug info */
        if (verbose_>1) {
            s->writeToFile("Outputs/spoiler.txt");
        }
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
            return recursive_negotiation(k,1-c,done+1,is_saturated);
        } else {
            /* compress the spoilers for component c, and update the current set of assumptions and guarantees */
            std::cout << "\tCompressing spoilers for component " << c << "." << '\n';
            negotiation::Spoilers spoiler(s);
            spoiler.boundedBisim(k);
            /* check saturation */
            if (spoiler.k_==k && is_saturated) {
                is_saturated=false;
            }
            /* update the guarantee of the other component */
            negotiation::SafetyAutomaton guarantee_updated(*guarantee_[1-c],*spoiler.spoilers_mini_);
            guarantee_updated.trim();
            /* determinize the guarantee (trimming might cause non-determinism) */
            guarantee_updated.determinize();
            /* minimize the guarantee automaton before saving */
            negotiation::Spoilers guarantee_final(&guarantee_updated);
            guarantee_final.boundedBisim();
            *guarantee_[1-c]=*guarantee_final.spoilers_mini_;
            /* save the current pair of guarantees */
            if (verbose_>1) {
                guarantee_[0]->writeToFile("Outputs/guarantee_0.txt");
                guarantee_[1]->writeToFile("Outputs/guarantee_1.txt");
            }
            bool flag2 = recursive_negotiation(k,1-c,0,is_saturated);
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
        /* debugging: print the number of sure and maybe winning states */
        if (verbose_>1) {
            int num_maybe=0;
            int num_sure=0;
            for (abs_type i=0; i<monitor.no_states; i++) {
                if (sure_safe[i]->size()!=0) {
                    num_sure++;
                }
                if (maybe_safe[i]->size()!=0) {
                    num_maybe++;
                }
            }
            std::cout << "\t\tNumber of sure safe states = " << num_sure << ".\n";
            std::cout << "\t\tNumber of maybe safe states = " << num_maybe << ".\n";
        }
        SafetyAutomaton* spoilers_safety = new SafetyAutomaton;
        int flag1 = monitor.find_spoilers(sure_safe, maybe_safe, spoilers_safety);
        /* if some initial states are surely losing the safety specification, then return out_flag=0 */
        if (flag1==0) {
            out_flag=0;
            return out_flag;
        }
        /* debuggig info */
        if (verbose_>1 && flag1==1) {
            std::cout << "\t\tSome initial states are not surely safe.\n";
        }
        spoilers_safety->trim();
        /* minimize the spoiler_safety automaton  */
        negotiation::Spoilers safety(spoilers_safety);
        safety.boundedBisim();
        /* find the spoilers for the liveness part (with the strategies being already restricted by the strategy obtained during the synthesis of the maybe safety controller) */
        SafetyAutomaton* spoilers_liveness = new SafetyAutomaton;
        /* assume that the liveness game is winning */
        int flag2=2;
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
        /* print debugging info */
        if (verbose_>1 && flag2==1) {
            std::cout << "\t\tSome initial states are not surely winning the liveness condition.\n";
        }
        /* if some initial states are surely losing the liveness specification, then return false */
        if (flag2==0) {
            out_flag=0;
            return out_flag;
        }
        spoilers_liveness->trim();
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
