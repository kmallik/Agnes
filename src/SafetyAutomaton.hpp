/* SafetyAutomaton.hpp
 *
 *  Created by: Kaushik
 *  Date: 16/11/2019  */

/** @file **/
#ifndef SAFETYAUTOMATON_HPP_
#define SAFETYAUTOMATON_HPP_

#include <vector>
#include <queue>

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
    /** @brief set of initial state indices **/
    std::unordered_set<abs_type> init_;
    /** @brief number of internal disturbance inputs P **/
    abs_type no_inputs_;
    /** @brief post set: post[(i-1)*P+j] contains the list of posts for state i and dist_input j **/
    std::unordered_set<abs_type>** post_;
public:
    /* copy constructor */
    SafetyAutomaton(const SafetyAutomaton& other) {
        no_states_=other.no_states_;
        init_=other.init_;
        no_inputs_=other.no_inputs_;
        post_ = new std::unordered_set<abs_type>*[no_states_*no_inputs_];
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
        abs_type ni;
        result = readMember<abs_type>(filename, ni, "NO_INITIAL_STATES");
        result = readSet<abs_type>(filename, init_, ni, "INITIAL_STATE_LIST");
        result = readMember<abs_type>(filename, no_inputs_, "NO_INPUTS");
        abs_type no_elems = no_states_*no_inputs_;
        post_ = new std::unordered_set<abs_type>*[no_elems];
        for (size_t i=0; i<no_elems; i++) {
            std::unordered_set<abs_type> *v=new std::unordered_set<abs_type>;
            post_[i]=v;
        }
        result = readArrSet<abs_type>(filename, post_, no_elems, "TRANSITION_POST");
        for (size_t i=0; i<no_elems; i++) {
            for (auto it=post_[i]->begin(); it!=post_[i]->end(); ++it) {
                if (*it >=no_states_) {
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
    void addPost(std::unordered_set<abs_type>** post) {
        /* now set the new post as the one supplied */
        int no_elems = no_states_*no_inputs_;
        post_ = new std::unordered_set<abs_type>*[no_elems];
        for (int i=0; i<no_elems; i++) {
            std::unordered_set<abs_type>* v=new std::unordered_set<abs_type>;
            post_[i]=v;
            for (auto it=post[i]->begin(); it!=post[i]->end(); ++it) {
                post_[i]->insert(*it);
            }
        }
    }
    /*! Determinize the safety automaton (using the universal accepting condition: the rejecting states are lumped into the one with index 0) */
    void determinize() {
        /* new deterministic post vector (will be converted to array later) */
        std::vector<abs_type> post_det;
        /* subsets of sets of states (encoded using decimal) already added, the index of set Q[i] in the deterministic automaton in i */
        std::vector<abs_type> Q;
        /* address of the binary encoding of set q in the vector Q, Q_inv[q]=2^no_states_ means that q is not present in Q at present */
        abs_type* Q_inv = new abs_type[(abs_type)pow(2,no_states_)];
        for (int i=0; i<pow(2,no_states_); i++) {
            Q_inv[i]=pow(2,no_states_);
        }
        /* initially the deterministic automaton has just two states: the reject state and the set of initial states */
        Q.push_back(0);
        Q_inv[0]=0;
        abs_type x0=decEnc(init_);
        Q.push_back(x0);
        Q_inv[x0]=1;
        /* set of subsets of states to be explored for post computation (processed in FIFO fashion) */
        std::queue<std::unordered_set<abs_type>*> E;
        std::unordered_set<abs_type>* s = new std::unordered_set<abs_type>;
        s->insert(0);
        E.push(s);
        E.push(&init_);
        while (E.size()!=0) {
            /* pop one set from E */
            std::unordered_set<abs_type>* pre = E.front();
            E.pop();
            for (abs_type j=0; j<no_inputs_; j++) {
                /* initialize a set of post states of pre */
                std::unordered_set<abs_type>* post = new std::unordered_set<abs_type>;
                for (auto i=pre->begin(); i!=pre->end(); ++i) {
                    std::unordered_set<abs_type>* post_set=post_[addr(*i,j)];
                    /* flag to check if any successor goes to reject */
                    bool unsafe=false;
                    for (auto k=post_set->begin(); k!=post_set->end(); ++k) {
                        if (*k==0) {
                            post->clear();
                            unsafe=true;
                            break;
                        } else {
                            post->insert(*k);
                        }
                    }
                    if (unsafe) {
                        break;
                    }
                }
                abs_type xx=decEnc(*post);
                if (Q_inv[xx]==pow(2,no_states_)) {
                    /* the set post has not been seen before */
                    Q.push_back(xx);
                    Q_inv[xx]=Q.size()-1;
                    E.push(post);
                }
                /* add the deterministic transition to the state index representing the post set */
                post_det.push_back(Q_inv[xx]);
            }
        }
        /* now update the class member variables */
        no_states_=Q.size();
        init_.clear();
        init_.insert(1);
        delete[] post_;
        post_ = new std::unordered_set<abs_type>*[post_det.size()];
        for (int i=0; i<post_det.size(); i++) {
            std::unordered_set<abs_type>* s=new std::unordered_set<abs_type>;
            s->insert(post_det[i]);
            post_[i]=s;
        }
    }
    /*! (Over-)write the safety automaton to a file */
    void writeToFile(const string& filename) {
        create(filename);
        writeMember(filename, "NO_STATES", no_states_);
        writeMember(filename, "NO_INITIAL_STATES", init_.size());
        writeSet(filename, "INITIAL_STATE_LIST", init_);
        writeMember<abs_type>(filename, "NO_INPUTS", no_inputs_);
        writeArrSet(filename,"TRANSITION_POST",post_, no_states_*no_inputs_);
    }
    /*! Decimal encoding of set of states
     * \param[in] S   set of states
     * \param[out] d    the decimal equivalent of the binary encoding of the states present in S (n-th bith is 0 if state n is not in S, and is 1 otherwise) */
    abs_type decEnc(const std::unordered_set<abs_type> S) {
        abs_type d = 0;
        for (abs_type i=0; i<no_states_; i++) {
            if (S.find(i)!=S.end()) {
                d+= pow(2,i);
            }
        }
        return d;
    }
    /*! Address of post in post array.
     * \param[in] i           state index
     * \param[in] j           input index
     * \param[out] ind    address of the post state vector in post **/
    inline int addr(const abs_type i, const abs_type j) {
        return (i*no_inputs_ + j);
    }
};/* end of class defintion */
}/* end of namespace negotiation */
#endif
