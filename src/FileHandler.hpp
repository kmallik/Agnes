/* FileHandler.hpp
 *
 *  Created by: Kaushik
 *  Date: 12/11/2019 */

#include <iostream>
#include <sstream>
#include <fstream>
#include <locale>

//#include "Functions.hpp"
/**
 *  @brief Some basic operations for reading inputs and writing output.
 */

template<class T>
int readMemberFromFile(const std::string& filename, T& member_value, const std::string& member_name) {
    std::ifstream file;
    file.open(filename);
    std::string line;
    if (file.is_open()) {
        while(std::getline(file,line)) {
            /* check if a match is found with the member_name */
            if(line.find(member_name)!=std::string::npos) {
                /* the data is in the next line*/
                if(std::getline(file,line)) {
                  std::istringstream stream(line);
                  stream >> member_value;
                } else {
                    std::cout << "Unable to read data member.\n";
                    return 0;
                }
                file.close();
                return 1;
            }
        }
        /* while loop exited and data member was not found */
        std::cout << "Member named " << member_name << " not found in file " << filename << ".\n";
        return 0;
    } else {
        std::cout << "Unable to open input file.\n";
        return 0;
    }
}

/* read 2-dimensional integer array from file */
template<class T>
int readVec(const std::string& filename, std::vector<T>& v, size_t no_elem, const std::string& vec_name) {
    std::ifstream file;
    file.open(filename);
    if (file.is_open()) {
        std::string line;
        while(std::getline(file,line)) {
            if(line.find(vec_name)!=std::string::npos) {
                for (size_t i=0; i<no_elem; i++) {
                    if(std::getline(file,line)) {
                        std::stringstream stream(line);
                        std::locale loc;
                        if (!std::isdigit(line[0],loc)) {
                            std::cerr << "Number of elements stored for " << vec_name << " in the file " << filename << " does not match with the number of state.\n";
                            return 0;
                        } else {
                            T val;
                            stream >> val;
                            v.push_back(val);
                        }
                    } else {
                        std::cout << "Unable to read vector.\n";
                        return 0;
                    }
                }
                file.close();
                return 1;
            }
        }
        /* while loop exited and the vec was not found */
        std::cout << "Vector named " << vec_name << " not found in file " << filename << ".\n";
        return 0;
    } else {
        std::cout << "Unable to open input file.\n";
        return 0;
    }
}
