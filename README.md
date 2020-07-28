# Agnes

Agnes is a tool for solving the distributed reactive synthesis problem. While the problem at hand is undecidable in general, Agnes promises to provide a sound but incomplete solution. The core underlying theory of Agnes can be found in our EMSOFT '20 paper.

Right now, Agnes is in its nascent stage: It can only support very simple architecture (two systems connected in feedback) with a restriction on the class of specifications (safety and deterministic Büchi).

# System Requirements

- Operating system: Linux, Max OS. Agnes has not be tested on Windows.
- A c++ development environment where where you can compile c++ source codes.
- Optional requirement for visualization of output: The 'dot' tool from Graphviz, which can be freely downloaded from the Graphviz website (https://www.graphviz.org/download/).
- Optional requirement for automatically generating the documentation: The Doxygen tool (can be freely downloaded from https://www.doxygen.nl/index.html).

# Installation

Agnes is written in header-only library style. You only need to add the Agnes source directory to the include directory in the compiler command.

# Directory Structure

In the repository, you will find the following directory structure:

- `./src/` Contains the C++ source code.
- `./doc/` The doxygen configuration file for automatically generating a documentation (requires Doxygen).
- `./examples/` Several examples.

# How to Use

