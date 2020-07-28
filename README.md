# Agnes

Agnes is a tool for automatically synthesizing controllers for distributed reactive systems. While the problem at hand is undecidable in general, Agnes promises to provide a sound but incomplete solution. The core underlying theory of Agnes can be found in our EMSOFT '20 paper; a preprint version is available in the following link: https://people.mpi-sws.org/~kmallik/uploads/AGDistSynth2020.pdf. To cite Agnes, please cite our EMSOFT '20 paper.

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

Coming soon.
