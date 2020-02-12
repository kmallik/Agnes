/*
 * DotInterface.hpp
 *
 *  Created on: 12.02.2020
 *      author: kaushik
 */

/*
 * A program for saving transition systems using the DOT language (for visualization)
 */

#include <fstream>

/*! Create a directed graph from a given transition array
 * \param[in] filename          the output filename
 * \param[in] graph_name        name of the graph
 * \param[in] vertex_labels     a map from each vertex to its corresponding display label
 * \param[in] edge_labels       a map from each input to its display label
 * \param[in] transitions       the transition array
 */
template<class T>
int createDiGraph(const std::string& filename,
                const std::string& graph_name,
                const std::vector<std::string*> vertex_labels,
                const std::vector<std::string*> edge_labels,
                std::unordered_set<T>** transitions) {
    /* open the file for writing */
    std::ofstream file;
    file.open(filename, std::ios_base::out);
    if (!file.is_open()) {
        try {
            throw std::runtime_error("FileHandler:writeSet: Unable to open input file.");
        } catch (std::exception &e) {
            std::cout << e.what() << "\n";
            return 0;
        }
    }
    /* the address of the post vector in the "transitions" array is determined using the following lambda expression */
    auto post_addr = [&](int x, int w) -> int {
        return (x*edge_labels.size() + w);
    };
    /* the object being created is a digraph */
    file << "digraph " << graph_name << " {"<< "\n";
    /* write all the transitions */
    for (int i=0; i<vertex_labels.size(); i++) {
        for (int j=0; j<edge_labels.size(); j++) {
            std::unordered_set<T> post_state=*transitions[post_addr(i,j)];
            for (auto i2=post_state.begin(); i2!=post_state.end(); ++i2) {
                file << "\t " << *vertex_labels[i] << " -> " << *i2 << " [label=\"" << *edge_labels[j] << "\"]\;" << "\n";
            }
        }
    }
    file << "}";
    file.close();
    return 1;
}
