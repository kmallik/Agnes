/* FileHandler.hpp
 *
 *  Created by: Kaushik
 *  Date: 12/11/2019 */

#include <iostream>
#include <sstream>
#include <fstream>
#include <locale>
#include <vector>

//#include "Functions.hpp"
/**
 *  @brief Some basic operations for reading inputs and writing output.
 */

template<class T>
int readMember(const std::string& filename, T& member_value, const std::string& member_name) {
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
                    try {
                        throw std::runtime_error("FileHandler:readMember: Unable to read data member");
                    } catch (std::exception &e) {
                        std::cout << e.what() << "\n";
                        return 0;
                    }
                }
                file.close();
                return 1;
            }
        }
        /* while loop exited and data member was not found */
        try {
            throw std::runtime_error("FileHandler:readMember: Member not found.");
        } catch (std::exception &e) {
            std::cout << e.what() << "\n";
            return 0;
        }
    } else {
        try {
            throw std::runtime_error("FileHandler:readMember: Unable to open input file.");
        } catch (std::exception &e) {
            std::cout << e.what() << "\n";
            return 0;
        }
    }
}

/* read 1-dimensional integer vector from file */
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
                            try {
                                throw std::runtime_error("FileHandler:readVec: Number of rows do not match with number of elements.");
                            } catch (std::exception &e) {
                                std::cout << e.what() << "\n";
                                return 0;
                            }
                        } else {
                            T val;
                            stream >> val;
                            v.push_back(val);
                        }
                    } else {
                        try {
                            throw std::runtime_error("FileHandler:readVec: Unable to read vector.");
                        } catch (std::exception &e) {
                            std::cout << e.what() << "\n";
                            return 0;
                        }
                    }
                }
                file.close();
                return 1;
            }
        }
        /* while loop exited and the vec was not found */
        try {
            throw std::runtime_error("FileHandler:readVec: Vector not found.");
        } catch (std::exception &e) {
            std::cout << e.what() << "\n";
            return 0;
        }
    } else {
        try {
            throw std::runtime_error("FileHandler:readVec: Unable to open input file.");
        } catch (std::exception &e) {
            std::cout << e.what() << "\n";
            return 0;
        }
    }
}

/* read vector of integer array (can be thought of as a 2-d table) from file */
template<class T, std::size_t SIZE>
int readVecArr(const std::string& filename, std::vector<std::array<T,SIZE>>& v, size_t no_elem, const std::string& vec_name) {
    std::ifstream file;
    file.open(filename);
    if (file.is_open()) {
        std::string line;
        /* go through all the lines until a match with vec_name is found */
        while(std::getline(file,line)) {
            if(line.find(vec_name)!=std::string::npos) {
                for (size_t i=0; i<no_elem; i++) {
                    if(std::getline(file,line)) {
                        std::stringstream stream(line);
                        std::locale loc;
                        if (!std::isdigit(line[0],loc)) {
                            try {
                                throw std::runtime_error("FileHandler:readVecArr: Number of rows do not match with number of elements.");
                            } catch (std::exception &e) {
                                std::cout << e.what() << "\n";
                                return 0;
                            }
                        } else {
                            std::array<T,SIZE> val;
                            for (size_t j=0; j<SIZE; j++) {
                                stream >> val[j];
                            }
                            v.push_back(val);
                        }
                    } else {
                        try {
                            throw std::runtime_error("FileHandler:readVecArr: Unable to read array.");
                        } catch (std::exception &e) {
                            std::cout << e.what() << "\n";
                            return 0;
                        }
                        
                    }
                }
                file.close();
                return 1;
            }
        }
        /* while loop exited and the vec was not found */
        try {
            throw std::runtime_error("FileHandler:readVecArr: Vector not found.");
        } catch (std::exception &e) {
            std::cout << e.what() << "\n";
            return 0;
        }
    } else {
        try {
            throw std::runtime_error("FileHandler:readVecArr: Unable to open input file.");
        } catch (std::exception &e) {
            std::cout << e.what() << "\n";
            return 0;
        }
    }
}

/* read array of vectors (can be thought of as a 2-d table) from file */
template<class T>
int readArrVec(const std::string& filename, std::vector<T>** arr, size_t no_elem, const std::string& arr_name) {
    std::ifstream file;
    file.open(filename);
    if (file.is_open()) {
        std::string line;
        /* go through all the lines until a match with arr_name is found */
        while(std::getline(file,line)) {
            if(line.find(arr_name)!=std::string::npos) {
                for (size_t i=0; i<no_elem; i++) {
                    if(std::getline(file,line)) {
//                        std::vector<T> vec;
//                        arr[i]=&vec;
                        if (line.compare("x")==0) {
                            continue;
                        } else {
                            std::stringstream line_stream(line);
                            while (line_stream.good()) {
                                T x;
                                line_stream >> x;
                                if (!line_stream.fail()) {
                                    arr[i]->push_back(x);
                                }
                            }
                        }
//                        std::stringstream stream(line);
//                        std::locale loc;
//                        if (!std::isdigit(line[0],loc)) {
//                            std::cerr << "Number of elements stored for " << arr_name << " in the file " << filename << " does not match with the number of state.\n";
//                            return 0;
//                        } else {
//
////                            do
////                                stream >> *arr[i];
////                            while
//                        }
                    } else {
                        try {
                            throw std::runtime_error("FileHandler:readArrVec: Unable to read vector.");
                        } catch (std::exception &e) {
                            std::cout << e.what() << "\n";
                            return 0;
                        }
                    }
                }
                file.close();
                return 1;
            }
        }
        /* while loop exited and the vec was not found */
        try {
            throw std::runtime_error("FileHandler:readArrVec: Array not found.");
        } catch (std::exception &e) {
            std::cout << e.what() << "\n";
            return 0;
        }
    } else {
        try {
            throw std::runtime_error("FileHandler:readArrVec: Unable to open input file.");
        } catch (std::exception &e) {
            std::cout << e.what() << "\n";
            return 0;
        }
    }
}
