# Agnes

Agnes is a tool for automatically synthesizing controllers for distributed reactive systems. While the problem at hand is undecidable in general, Agnes promises to provide a sound but incomplete solution. The core underlying theory of Agnes can be found in our EMSOFT 2020 paper; a preprint version is available in the following link: https://people.mpi-sws.org/~kmallik/uploads/AGDistSynth2020.pdf. To cite Agnes, please cite our EMSOFT 2020 paper.

Right now, Agnes is in its nascent stage: It can only support very simple architecture (two systems connected in feedback) with a restriction on the class of specifications (safety and deterministic Büchi).

## System Requirements

- Operating system: Linux, Max OS. Agnes has not be tested on Windows.
- A C++ development environment where where you can compile C++ source codes.
- Optional requirement for visualization of output: The 'dot' tool from Graphviz, which can be freely downloaded from the Graphviz website (https://www.graphviz.org/download/).
- Optional requirement for automatically generating the documentation: The Doxygen tool (can be freely downloaded from https://www.doxygen.nl/index.html).

## Installation

- Agnes is written in header-only library style. You only need to add the Agnes source directory to the include directory in the compiler command.
- The documentation can be automatically generated from the command line by navigating to the `<Agnes root>/doc/` folder, and executing the following command: 

    > doxygen doxygen-config-file
  
  This requires the Doxygen tool to be installed first. This will create two subfolders `<Agnes root>/doc/html/` and `<Agnes root>/doc/latex/`, and the documentation can be opened in either of the following ways:
  + Open the file `<Agnes root>/doc/html/index.html` in a browser.
  + First execute `make` from the command line in the folder `<Agnes root>/doc/latex/` and then open the file `<Agnes root>/doc/latex/refman.pdf` in a pdf-viewer.

## Directory Structure

In the repository, you will find the following directory structure:

- `./src/` Contains the C++ source code.
- `./doc/` The doxygen configuration file for automatically generating a documentation (requires Doxygen).
- `./examples/factory-parameterized` The parameterized tandem queuing netowork example from our EMSOFT 2020 paper.
- `./examples/mutex-parameterized` The parameterized distributed packet sending example from our EMSOFT 2020 paper.

## How to Use

> A more user-friendly command-line version is coming soon that would automate most of the following steps. Until then, you can use Agnes as an API to write your own code for syntehsizing controllers for distributed reactive systems.

If you want to use Agnes to synthesize controllers for your own distributed synthesis problem, you have follow the following steps:

1. Create the following text files to specify the inputs:

    - Two text files, call them `system_0.txt` and `system_1.txt` for encoding the description of the two systems. Each of these two files contain the attributes (e.g. the number of states, number of inputs, transition matrix etc.) of the two systems. Each attribute is specified by first specifying the attribute's name, and then specifying the attribute's value (scalar/1-d array/2-d array) in the following line(s). Following is the list of attributes with their value types in brackets:
        + NO_STATES (scalar): Number of system states. If a system has N states, then they are indexed 0,...,N-1.
        + NO_INITIAL_STATES (scalar): Number of initial states.
        + INITIAL_STATE_LIST (1-d array): A list of the initial state indices.
        + NO_CONTROL_INPUTS (scalar): Number of control inputs. If a system has M control inputs, then they are indexed 0,...,M-1.
        + NO_DIST_INPUTS (scalar): Number of disturbance inputs coming from the other system's output. Note that the external enviornmental disturbances are implicitly modeled using nondeterministic transitions. If a system has P disturbance inputs, then they are indexed 0,...,P-1.
        + NO_OUTPUTS (scalar): Number of outputs. If a system has L outputs, then the outputs are indexed 0,...,L-1.
        + STATE_TO_OUTPUT (1-d array): A list of size N that maps each i-th state to its corresponding output index.
        + TRANSITION_POST (2-d array): An 1-d array having N\*M\*P array elements, where each element itself can be an 1-d array. The (i\*M\*P + j\*P + k)-th element is an 1-d array containing indices of all the successor states from the state i, when the control input j and the disturbance input k are applied.
    - Two text files, call them `safe_states_0.txt` and `safe_states_1.txt`, for encoding the safe states of the two systems. Each of these files contain the following attributes along with their corresponding values in the following line(s):
        + NO_SAFE_STATES (scalar): Number of safe states. Must not be greater than the number of the system's states.
        + SET_SAFE_STATES (1-d array): The list of state indices which are safe. The size of the list must be equal to the value of the attribute NO_SAFE_STATES.
    - Two text files, call them `target_states_0.txt` and `target_states_1.txt`, for encoding the set of target states (for Büchi specification) of the two systems. Each of these files contain the following attributes along with their corresponding values in the following line(s):
        + NO_TARGET_STATES (scalar): Number of target states. Must not be greater than the number of the system's states.
        + SET_TARGET_STATES (1-d array): The list of state indices which are in the target set. The size of the list must be equal to the value of the attribute NO_TARGET_STATES.
    
2. The distributed synthesis problem can be solved by writing a C++ program that executes the following instructions:
    
        negotiation::Negotiate negotiation_object(systems, safe_states, target_states, k_max);
        int k = negotiation_object.iterative_deepening_search();
    
   where `systems` is a vector that contains the system file names `system_0.txt` and `system_1.txt`, `safe_states` is a vector that contains the filenames `safe_states_0.txt` and `safe_states_.txt` specifying the safe states, `target_states` is a vector that contains the filenames `target_states_0.txt` and `target_states_1.txt` specifying the target states, `k_max` is an optional argument specifying the maximum length of the patterns used for under-approximaing the contracts (see the EMSOFT paper), and `k` is the actual length of pattern (not greater than `k_max`) for which a solution could be found.
   
3. The output can be stored by executing the following isntructions:

        negotiation_object.guarantee_[0]->writeToFile('guarantee_0.txt');
        negotiation_object.guarantee_[1]->writeToFile('guarantee_1.txt');
    
   This will save the guarantee of each individual system (same as the assumption of the other system) in text files in the form of prefix-closed deterministic finite automata over the output alphabet of the respective system.

4. If Graphviz is installed, the guarantees can be additionally visualized for convenience. For this, the following instructions need to be executed in the C++ program:

        negotiation_object.guarantee_[0]->createDOT('file_0.gv', 'guarantee_automaton_0.txt', list_output_labels_1);
        negotiation_object.guarantee_[1]->createDOT('file_1.gv', 'guarantee_automaton_1.txt', list_output_labels_0);
    
   where `list_output_labels_i` for `i \in {0,1}` are the sets of string labels assigned to the outputs of the i-th system. The above instructions will create the files `file_0.gv` and `file_1.gv`, so that running 'dot' on these files would create the visualizations the two guarantee automata in desired format; see the Graphviz 'dot' documentation for details of the usage (https://www.graphviz.org/pdf/dotguide.pdf).

## Instructions for Repeating the Experiments from Our EMSOFT 2020 Paper

We presented two experiments in our EMSOFT 2020 paper: (a) a paramterized version of a distributed packet sending problem and (b) a parameterized version of a distributed tandem queueing network problem. Both of these examples can be found in the folder `<Agnes root>/examples/`. Table I and Table II from the EMSOFT 2020 paper can be generated by performing the following steps:

   1. Let us use the variable `EXAMPLE` to denote the respective example: For Table I, `EXAMPLE=mutex` and for Table II, `EXAMPLE=factory`. Open `<Agnes root>/examples/EXAMPLE-parameterized/EXAMPLE-generate/EXAMPLE-generate.cpp` and adjust the parameters.
   2. Use the command line to navigate to the folder `<Agnes root>/examples/EXAMPLE-parameterized/EXAMPLE-generate/`.
   3. Execute the following instructions to create a specific instance of the example for the specified parameters:
    
    make clean
    make
    ./EXAMPLE-generate
    
   4. In the command line, navigate to the newly created folder `<Agnes root>/examples/EXAMPLE-parameterized/EXAMPLE_<parameter values>/`. Execute the following instructions to solve the synthesis problem:
   
    make clean
    make
    ./EXAMPLE
    
   This will also append the results to the log files `<Agnes root>/examples/EXAMPLE-parameterized/results_iterative_search.log` and `<Agnes root>/examples/EXAMPLE-parameterized/results_plain_negotiation.log`, which contain respectively the results of the negotiation when the pattern-based under-approximation heuristic was enabled to minimize the size of the contracts, and when the same was disabled.
