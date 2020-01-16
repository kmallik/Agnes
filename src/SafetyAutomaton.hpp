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
            std::unordered_set<abs_type>* p=new std::unordered_set<abs_type>;
            *p=*other.post_[i];
            post_[i]=p;
        }
    }
    /*! Default constructor */
    SafetyAutomaton() {
        no_states_=0;
        no_inputs_=0;
    }
    /*! Constructor: all strings are accepted.
     * \param[in] no_dist_inputs        number of disturbance inputs. */
    SafetyAutomaton(const abs_type no_dist_inputs) {
        /* two states: 0 is non-accepting and 1 is accepting */
        no_states_=2;
        /* state 1 is initial */
        init_.insert(1);
        no_inputs_=no_dist_inputs;
        std::unordered_set<abs_type>** post = new std::unordered_set<abs_type>*[no_states_*no_inputs_];
        for (abs_type i=0; i<no_states_; i++) {
            for (abs_type j=0; j<no_inputs_; j++) {
                /* add self loop to state i on input j */
                std::unordered_set<abs_type>* s=new std::unordered_set<abs_type>;
                s->insert(i);
                post[addr(i,j)]=s;
            }
        }
        addPost(post);

        delete[] post;
    }
    /*! Constructor: product of two deterministic safety automata
     * \param[in] A1      the first deterministic safety automaton
     * \param[in] A2      the second deterministic safety automaton */
    SafetyAutomaton(const negotiation::SafetyAutomaton& A1,
                 const negotiation::SafetyAutomaton& A2) {
        /* the number of states of the product is the product of the number of states of A1 and A2 */
        // no_states_ = A1.no_states_*A2.no_states_;
        no_states_ = (A1.no_states_-1)*(A2.no_states_-1)+1;
        /* the new state index is derived using the following lambda expression */
        auto new_ind = [&](abs_type i1, abs_type i2) -> abs_type {
            if (i1==0 || i2==0) {
                /* whenever one of the individual states is non-accepting, the joint state is also non-accepting */
                return 0;
            } else {
                /* the indexing starts at 1 (excluding the sink) */
                return ((i1-1)*(A2.no_states_-1) + (i2-1)) + 1;
            }
            // return (i1*A2.no_states_ + i2);
        };
        /* a product state is initial if all the corresponding individual states are initial */
        for (auto i1=A1.init_.begin(); i1!=A1.init_.end(); ++i1) {
            for (auto i2=A2.init_.begin(); i2!=A2.init_.end(); ++i2) {
                init_.insert(new_ind(*i1,*i2));
            }
        }
        /* sanity check: the size of input space of A1 and A2 should be the same */
        if (A1.no_inputs_!=A2.no_inputs_) {
            try {
                throw std::runtime_error("SafetyAutomaton: product: the input spaces of the individual automata do not match.");
            } catch (std::exception& e) {
                std::cout << e.what();
            }
        } else {
            no_inputs_=A1.no_inputs_;
        }
        /* compute the post */
        std::unordered_set<abs_type>** post = new std::unordered_set<abs_type>*[no_states_*no_inputs_];
        /* first add self loops to the sink state (state index 0) */
        for (abs_type j=0; j<no_inputs_; j++) {
            std::unordered_set<abs_type>* set = new std::unordered_set<abs_type>;
            set->insert(0);
            post[addr(0,j)]=set;
        }
        /* compute post for the non-sink states */
        abs_type index = 2;
        for (abs_type i1=1; i1<A1.no_states_; i1++) {
            for (abs_type i2=1; i2<A2.no_states_; i2++) {
                abs_type i_new = new_ind(i1,i2);
                for (abs_type j=0; j<no_inputs_; j++) {
                    std::unordered_set<abs_type>* set = new std::unordered_set<abs_type>;
                    abs_type p1 = A1.addr(i1,j);
                    for (auto l1=A1.post_[p1]->begin(); l1!=A1.post_[p1]->end(); ++l1) {
                        abs_type p2 = A2.addr(i2,j);
                        for (auto l2=A2.post_[p2]->begin(); l2!=A2.post_[p2]->end(); ++l2) {
                            set->insert(new_ind(*l1,*l2));
                        }
                    }
                    post[index]=set;
                    index++;
                }
            }
        }
        addPost(post);
        delete[] post;
    }
//    /*! Check emptiness.
//     * \param[out] true/false   empty/non-empty*/
//    bool isEmpty() {
//
//    }
    /*! the equality operator creates new array for post_ */
    SafetyAutomaton& operator=(const SafetyAutomaton& other) {
        no_states_=other.no_states_;
        init_=other.init_;
        no_inputs_=other.no_inputs_;
        post_ = new std::unordered_set<abs_type>*[no_states_*no_inputs_];
        for (int i=0; i<no_states_*no_inputs_; i++) {
            std::unordered_set<abs_type>* p=new std::unordered_set<abs_type>;
            *p=*other.post_[i];
            post_[i]=p;
        }
        return *this;
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
    /*! Overwrite the post array.
     * \param[in] post      the new post array */
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
    /*! Trim the unreachable part of a safety automaton.
     *  The result is used to update the safety automaton. */
    void trim() {
        /* compute the set of reachable states */
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
            for (abs_type j=0; j<no_inputs_; j++) {
                /* address in the post array */
                abs_type post_addr = addr(i,j);
                for (auto i2=post_[post_addr]->begin(); i2!=post_[post_addr]->end(); ++i2) {
                    /* if the state i2 is not seen, then add i2 to the queue and seen */
                    if (seen.find(*i2)==seen.end()) {
                        fifo.push(*i2);
                        seen.insert(*i2);
                    }
                }
            }
        }
        /* the set seen is the set of reachable states, and serves as the new state space */
        std::vector<abs_type> new_to_old;
        abs_type* old_to_new = new abs_type[no_states_];
        /* the new reject state is mapped to the old reject state */
        new_to_old.push_back(0);
        old_to_new[0]=0;
        for (auto i=seen.begin(); i!=seen.end(); ++i) {
            if (*i!=0) {
                new_to_old.push_back(*i);
                old_to_new[*i]=new_to_old.size()-1;
            }
        }
        /* update number of states */
//        abs_type no_states_old=no_states_;
        no_states_=new_to_old.size();
        /* update the initial state*/
        std::unordered_set<abs_type> init_old=init_;
        init_.clear();
        for (auto i=init_old.begin(); i!=init_old.end(); ++i) {
            init_.insert(old_to_new[*i]);
        }
        /* number of inputs remain the same: one side-effect of this is that we can use the function addr for computing both the new and old address of post in the post array. */
        /* update the post array */
        std::unordered_set<abs_type>** post_new = new std::unordered_set<abs_type>*[no_states_*no_inputs_];
        for (abs_type i_new=0; i_new<no_states_; i_new++) {
            abs_type i_old=new_to_old[i_new];
            for (abs_type j=0; j<no_inputs_; j++) {
                abs_type addr_post_old=addr(i_old,j);
                abs_type addr_post_new=addr(i_new,j);
                std::unordered_set<abs_type>* set = new std::unordered_set<abs_type>;
                for (auto i2=post_[addr_post_old]->begin(); i2!=post_[addr_post_old]->end(); ++i2) {
                    set->insert(old_to_new[*i2]);
                }
                post_new[addr_post_new]=set;
            }
        }
        addPost(post_new);
        delete[] post_new;
        delete[] old_to_new;
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
    inline int addr(const abs_type i, const abs_type j) const {
        return (i*no_inputs_ + j);
    }
};/* end of class defintion */
}/* end of namespace negotiation */
#endif
