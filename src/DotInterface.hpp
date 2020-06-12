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

/*! Create a simple directed graph from a given transition array
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
                const std::unordered_set<T> init_vertices,
                const std::vector<std::string*> edge_labels,
                std::unordered_set<T>** transitions) {
    /* open the file for writing */
    std::ofstream file;
    file.open(filename, std::ios_base::out);
    if (!file.is_open()) {
        try {
            throw std::runtime_error("DotInterface:createDiGraph: Unable to open input file.");
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
    for (size_t i=0; i<vertex_labels.size(); i++) {
        for (size_t j=0; j<edge_labels.size(); j++) {
            std::unordered_set<T> post_state=*transitions[post_addr(i,j)];
            for (auto i2=post_state.begin(); i2!=post_state.end(); ++i2) {
                file << "\t " << *vertex_labels[i] << " -> " << *vertex_labels[*i2] << " [label=\"" << *edge_labels[j] << "\"];" << "\n";
            }
        }
    }
    /* the initial vertices appear as diamond shaped */
    for (auto i=init_vertices.begin(); i!=init_vertices.end(); ++i) {
        file << "\t" << *i << " [shape=diamond]" << "\n";
    }
    file << "}";
    file.close();
    return 1;
}
/*! Create a directed graph with vertex clusters from a given transition array
 * \param[in] filename          the output filename
 * \param[in] graph_name        name of the graph
 * \param[in] vertex_labels     a map from each vertex to its corresponding display label
 * \param[in] vertex_clusters   a list of set of vertices which appear as a cluster (the ones which are not listed in any of the cluster appear as single isolated vertices)
 * \param[in] edge_labels       a map from each input to its display label
 * \param[in] transitions       the transition array
 */
template<class T>
int createDiGraph(const std::string& filename,
                const std::string& graph_name,
                const std::vector<std::string*> vertex_labels,
                const std::vector<std::unordered_set<T>*> vertex_clusters,
                const std::unordered_set<T> init_vertices,
                const std::vector<std::string*> edge_labels,
                std::unordered_set<T>** transitions) {
    /* sanity check: the sets appearing in the cluster should be mutually exclusive */
    for (auto c1=vertex_clusters.begin(); c1!=vertex_clusters.end(); ++c1) {
        for (auto c2=vertex_clusters.begin(); c2!=vertex_clusters.end(); ++c2) {
            /* if any c1 and c2, when c1!=c2, have non-empty intersection, then throw an error */
            for (auto i1=(*c1)->begin(); i1!=(*c1)->end(); ++i1) {
                if (**c1==**c1) {
                    continue;
                }
                if ((*c2)->find(*i1)!=(*c2)->end()) {
                    try {
                        throw std::runtime_error("DotInterface:createDiGraph: the vertex clusters are not mutually exclusive");
                    } catch (std::exception &e) {
                        std::cout << e.what() << "\n";
                        return 0;
                    }
                }
            }
        }
    }
    /* open the file for writing */
    std::ofstream file;
    file.open(filename, std::ios_base::out);
    if (!file.is_open()) {
        try {
            throw std::runtime_error("DotInterface:createDiGraph: Unable to open input file.");
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
    /* first create all the subgraphs and remember which vertices already appeared there */
    std::unordered_set<T> vertices_in_clusters;
    int num_clusters=0;
    for (auto c=vertex_clusters.begin(); c!=vertex_clusters.end(); ++c) {
        file << "\tsubgraph cluster" << num_clusters << " \{\n";
        file << "\t\tnode [style=filled];\n";
        for (auto i=(*c)->begin(); i!=(*c)->end(); ++i) {
            /* bookkeeping */
            vertices_in_clusters.insert(*i);
            /* add outgoing transitions for the current vertex */
            for (size_t j=0; j<edge_labels.size(); j++) {
                std::unordered_set<T> post_state=*transitions[post_addr(*i,j)];
                for (auto i2=post_state.begin(); i2!=post_state.end(); ++i2) {
                    file << "\t\t " << *vertex_labels[*i] << " -> " << *vertex_labels[*i2] << " [label=\"" << *edge_labels[j] << "\"];" << "\n";
                }
            }
        }
        file << "\t}\n";
        num_clusters++;
    }
    /* write all the transitions for states not appearing in clusters */
    for (size_t i=0; i<vertex_labels.size(); i++) {
        if (vertices_in_clusters.find(i)!=vertices_in_clusters.end()) {
            continue;
        }
        for (size_t j=0; j<edge_labels.size(); j++) {
            std::unordered_set<T> post_state=*transitions[post_addr(i,j)];
            for (auto i2=post_state.begin(); i2!=post_state.end(); ++i2) {
                file << "\t " << *vertex_labels[i] << " -> " << *vertex_labels[*i2] << " [label=\"" << *edge_labels[j] << "\"];" << "\n";
            }
        }
    }
    /* the initial vertices appear as diamond shaped */
    for (auto i=init_vertices.begin(); i!=init_vertices.end(); ++i) {
        file << "\t" << *i << " [shape=diamond]" << "\n";
    }
    file << "}";
    file.close();
    return 1;
}
