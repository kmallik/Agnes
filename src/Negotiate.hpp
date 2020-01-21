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
        int k_act;
        /* the flag which is true if a solution is reached */
        bool success;
        /* negotiate until either a solution is found, or until increasing k does not change anything */
        while (1) {
            /* stop when maximum depth is reached */
            if (k==max_depth_) {
                std::cout << "Maximum search depth of spoiling behavior reached. No solution found. Terminating." << '\n';
            }
            /* index of the component starting negotiation: inconsequential to the outcome */
            int starting_component=0;
            /* number of components which can surely win so far: initially 0 */
            int done=0;
            /* recursively perform the negotiation */
            std::cout << "current depth = " << k << std::endl;
            bool success = recursive_negotiation(k,k_act,starting_component,done);
            if (k_act<k) {
                std::cout << "The search of spoiling behavior got saturated. No solution found. Terminating." << '\n';
                return false;
            }
            if (success) {
                return true;
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
    /*! Perform negotiation recursively with fixed length of spoiling behavior.
     * \param[in] k         prescribed length of the spoiling behavior set.
     * \param[in] k_act     actual length of the spoiling behavior used (could be smaller than k_max due to saturation of the bounded bisimulation algorithm).
     * \param[in] c         the index of the component who gets to compute the spoiling behaviors in this round.
     * \param[in] done  a counter counting the number of components which can surely win with the current contracts.
     * \param[out] true/false   success/failure of the negotiation. */
    bool recursive_negotiation(const int k, int& k_act, const int c, int done) {
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
        int flag = compute_spoilers_overall(c,s);
        /* debug */
//        s->writeToFile("Outputs/interim_overall_det.txt");
        /* debug ends */
        if (flag==0) {
            /* when the game is sure losing for component c, the negotiation fails */
            std::cout << "\tThe game is sure losing for component " << c << ". The negotiation failed. Terminating the process." << '\n';
            return false;
        } else if (done==2) {
            /* when both the components have sure winning strategies with the current set of contracts, the negotiation successfully terminates */
            std::cout << "\tThe game is sure winning for both of the components. The negoitaiotn succeeded. Terminating the process." << '\n';
            return true;
        } else if (flag==2) {
            /* when the component c---but not (1-c)---has a sure winning strategy with the present contract, it's component (1-c)'s turn to compute the spoilers */
            std::cout << "\tThe game is sure winning for component " << c << "." << '\n';
            return recursive_negotiation(k,k_act,1-c,done+1);
        } else {
            /* find the spoilers for component c, and update the current set of assumptions and guarantees */
            std::cout << "\tComputing spoilers for component " << c << "." << '\n';
            negotiation::Spoilers spoiler(s);
            spoiler.boundedBisim(k);
            k_act=spoiler.k_;
            // /* debug */
            // spoiler.spoilers_mini_->writeToFile("Outputs/interim_overall_mini.txt");
            // /* debug ends */
            negotiation::SafetyAutomaton guarantee_updated(*guarantee_[1-c],*spoiler.spoilers_mini_);
            *guarantee_[1-c]=guarantee_updated;
            // /* save the updated guarantee */
            // Str += "Outputs/G";
            // Str += std::to_string(1-c);
            // Str += ".txt";
            // char Char[20];
            // size_t Length = Str.copy(Char, Str.length() + 1);
            // Char[Length] = '\0';
            // guarantee_[1-c]->writeToFile(Char);
            // /* debug */
            // guarantee_[1-c]->writeToFile("Outputs/interim_updated_guarantee.txt");
            // /* debug ends */
            bool flag2 = recursive_negotiation(k,k_act,1-c,0);
            if (flag2) {
                return true;
            } else {
                return false;
            }
        }
    }
    /*! Find the overall spoiling behavior for a given component in the form of a safety automaton.
     * \param[in] c                      the component index
     * \param[in] spoilers      the pointer to the safety automaton storing the spoiling behaviors
     * \param[out] out_flag   0 -> some initial states are sure losing, 2 -> all initial states are sure winning, 1-> otherwise. */
    int compute_spoilers_overall(const int c, negotiation::SafetyAutomaton* spoilers) {
        /* the output flag */
        int out_flag;
        /* find the spoilers for the safety part */
        negotiation::SafetyGame monitor(*components_[c],*guarantee_[1-c],*guarantee_[c]);
        // /* debug */
        // monitor.writeToFile("Outputs/monitor.txt");
        // /* debug end */
        std::vector<std::unordered_set<abs_type>*> sure_safe = monitor.solve_safety_game(*safe_states_[c],"sure");
        std::vector<std::unordered_set<abs_type>*> maybe_safe = monitor.solve_safety_game(*safe_states_[c],"maybe");
        SafetyAutomaton* spoilers_safety = new SafetyAutomaton;
        int flag1 = monitor.find_spoilers(sure_safe, maybe_safe, spoilers_safety);
        /* if some initial states are surely losing the safety specification, then return out_flag=0 */
        if (flag1==0) {
            out_flag=0;
            return out_flag;
        }
        /* debugging */
//        spoilers_safety->writeToFile("Outputs/interim_safe.txt");
        /* end of debugging */
        spoilers_safety->trim();
        spoilers_safety->determinize();
        // /* debugging */
        // spoilers_safety->writeToFile("Outputs/interim_safe_det.txt");
        // /* end of debugging */
        // /* construct a monitor for the liveness game which has the same state space as the safety monitor, and the transitions are obtained by deleting the transitions in the safety monitor which are disallowed by the control strategy */
        // /* the allowed inputs are all the possible control inputs */
        // std::vector<std::unordered_set<abs_type>*> allowed_inputs;
        // for (abs_type i=0; i<monitor.no_states; i++) {
        //     std::unordered_set<abs_type>* s = new std::unordered_set<abs_type>;
        //     for (abs_type j=0; j<monitor.no_control_inputs; j++) {
        //         s->insert(j);
        //     }
        //     allowed_inputs.push_back(s);
        // }
        /* find the spoilers for the liveness part (with the strategies being already restricted by the strategy obtained during the synthesis of the maybe safety controller) */
        SafetyAutomaton* spoilers_liveness = new SafetyAutomaton;
        // std::vector<std::unordered_set<abs_type>*> allowed_inputs;
        // for (abs_type i=0; i<monitor.no_states; i++) {
        //     std::unordered_set<abs_type>* s = new std::unordered_set<abs_type>;
        //     for (auto j=sure_safe[i]->begin(); j!=sure_safe[i]->end(); ++j) {
        //         s->insert(*j);
        //     }
        //     for (auto l=maybe_safe[i]->begin(); l!=maybe_safe[i]->end(); ++l) {
        //         abs_type j= components_[c]->cont_ind(*l);
        //         s->insert(j);
        //     }
        //     allowed_inputs.push_back(s);
        // }
        negotiation::LivenessGame monitor_live(*components_[c], *guarantee_[1-c], *guarantee_[c], *target_states_[c], sure_safe, maybe_safe);
        // /* here the assumption is that the state space of monitor_safe and of monitor_live are the same (because allowed_inputs uses the same state space) */
        // // negotiation::LivenessGame monitor_live(*components_[c], *guarantee_[1-c], *guarantee_[c], *target_states_[c], allowed_inputs);
        // /* new: the assumption is updated with the spoilers from the safety part */
        // negotiation::SafetyAutomaton new_assumption(*spoilers_safety, *guarantee_[1-c]);
        // new_assumption.determinize();
        // new_assumption.trim();
        // negotiation::LivenessGame monitor_live(*components_[c], new_assumption, *guarantee_[c], *target_states_[c], allowed_inputs);
        // /* debug */
        // monitor_live.writeToFile("Outputs/monitor_live.txt");
        // /* debug end */
        int flag2 = monitor_live.find_spoilers(spoilers_liveness);
        /* if some initial states are surely losing the liveness specification, then return false */
        if (flag2==0) {
            out_flag=0;
            return out_flag;
        }
//         /* debug */
//         spoilers_liveness->writeToFile("Outputs/interim_live.txt");
//        spoilers_safety->writeToFile("Outputs/interim_safe.txt");
//         /* debug ends */
        spoilers_liveness->trim();
        spoilers_liveness->determinize();
        // /* debug */
        // spoilers_liveness->writeToFile("Outputs/interim_live_det.txt");
        // /* debug ends */
        /* the overall spoiling behavior is the union of spoiling behavior for the safety spec and the liveness spec, or the overall non-spoiling behavior is the intersection of non-spoilers for safety AND non-spoilers for liveness */
        SafetyAutomaton spoilers_overall(*spoilers_safety, *spoilers_liveness);
        spoilers_overall.trim();

        /* copy the overall spoiling behavior to the one supplied as input for storing the spoiling behaviors */
        *spoilers=spoilers_overall;
        /* if both the safety and the liveness games are sure winning, then return out_flag=2, else return out_flag=1 */
        if (flag1==2 && flag2==2) {
            out_flag=2;
        } else {
            out_flag=1;
        }
        return out_flag;
    }
}; /* end of class definition */
} /* end of namespace negotiation */
#endif
