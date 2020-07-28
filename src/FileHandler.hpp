/* FileHandler.hpp
 *
 *  Created by: Kaushik
 *  Date: 12/11/2019 */

#include <iostream>
#include <sstream>
#include <fstream>
#include <locale>
#include <vector>
#include <unordered_set>
#include <dirent.h>
#include <sys/stat.h>
#include <cstring>

/**
 *  @brief Some basic operations for reading inputs and writing output.
 */

 /*! Ensures that a specified subdirectory exists.
  *  \param[in]  dirName     Name of the desired subdirectory.
  */
 template <class dir_type>
 void checkMakeDir(dir_type dirName) {
     DIR* dir = opendir(dirName);
     if (dir) {
         closedir(dir);
     }
     else if (ENOENT == errno) {
         int result = mkdir(dirName, 0777);
         (void) result;
     }
 }

/*! Read a member from a file. (A member is an attribute whose value is a scalar.)
 * \param[in] filename  Name of the file
 * \param[in] member_value  Reference to the variable that will contain the read member value
 * \param[in] member_name     The name of the member whose value is to be read
 * \param[out] out_flag          A flag that is 0 when reading was unsuccessful, and is 1 when reading was successful*/
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

/*! Read 1-dimensional vector from file
 * \param[in] filename  Name of the file
 * \param[in] v                  Reference to the vector that will contain the read vector value
 * \param[in] no_elem     The size of the vector
 * \param[in] vec_name     The name of the vector whose value is to be read
 * \param[out] out_flag          A flag that is 0 when reading was unsuccessful, and is 1 when reading was successful*/
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

/*! Read vector of pointers from file
 * \param[in] filename  Name of the file
 * \param[in] v                  Reference to the vector that will contain the read vector value
 * \param[in] no_elem     The size of the vector
 * \param[in] vec_name     The name of the vector whose value is to be read
 * \param[out] out_flag          A flag that is 0 when reading was unsuccessful, and is 1 when reading was successful*/
template<class T>
int readVec(const std::string& filename, std::vector<T*>& v, size_t no_elem, const std::string& vec_name) {
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
                        // if (!std::isdigit(line[0],loc)) {
                        //     try {
                        //         throw std::runtime_error("FileHandler:readVec: Number of rows do not match with number of elements.");
                        //     } catch (std::exception &e) {
                        //         std::cout << e.what() << "\n";
                        //         return 0;
                        //     }
                        // } else {
                            T* val=new T;
                            stream >> *val;
                            v.push_back(val);
                        // }
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

/*! Read 1-dimensional integer set (unordered) from file
 * \param[in] filename  Name of the file
 * \param[in] s                  Reference to the set that will contain the read set value
 * \param[in] no_elem     The size of the set
 * \param[in] set_name     The name of the set whose value is to be read
 * \param[out] out_flag          A flag that is 0 when reading was unsuccessful, and is 1 when reading was successful*/
template<class T>
int readSet(const std::string& filename, std::unordered_set<T>& s, size_t no_elem, const std::string& set_name) {
    std::ifstream file;
    file.open(filename);
    if (file.is_open()) {
        std::string line;
        while(std::getline(file,line)) {
            if(line.find(set_name)!=std::string::npos) {
                for (size_t i=0; i<no_elem; i++) {
                    if(std::getline(file,line)) {
                        std::stringstream stream(line);
                        std::locale loc;
                        if (!std::isdigit(line[0],loc)) {
                            try {
                                throw std::runtime_error("FileHandler:readSet: Number of rows do not match with number of elements.");
                            } catch (std::exception &e) {
                                std::cout << e.what() << "\n";
                                return 0;
                            }
                        } else {
                            T val;
                            stream >> val;
                            s.insert(val);
                        }
                    } else {
                        try {
                            throw std::runtime_error("FileHandler:readSet: Unable to read set.");
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
        /* while loop exited and the set was not found */
        try {
            throw std::runtime_error("FileHandler:readSet: Set not found.");
        } catch (std::exception &e) {
            std::cout << e.what() << "\n";
            return 0;
        }
    } else {
        try {
            throw std::runtime_error("FileHandler:readSet: Unable to open input file.");
        } catch (std::exception &e) {
            std::cout << e.what() << "\n";
            return 0;
        }
    }
}

/*! Read vector of arrays (can be thought of as a 2-d table) from file
 * \param[in] filename  Name of the file
 * \param[in] v                  Reference to the vector that will contain the read vector value
 * \param[in] no_elem     The size of the vector
 * \param[in] vec_name     The name of the vector whose value is to be read
 * \param[out] out_flag          A flag that is 0 when reading was unsuccessful, and is 1 when reading was successful*/
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

/*! Read vector of pointers to unordered sets (can be thought of as a 2-d table) from file
 * \param[in] filename  Name of the file
 * \param[in] vec             Reference to the vector that will contain the read vector value
 * \param[in] no_elem     The size of the vector
 * \param[in] vec_name     The name of the vector whose value is to be read
 * \param[out] out_flag          A flag that is 0 when reading was unsuccessful, and is 1 when reading was successful*/
template<class T>
int readVecSet(const std::string& filename, std::vector<std::unordered_set<T>*>& vec, size_t no_elem, const std::string& vec_name) {
    std::ifstream file;
    file.open(filename);
    if (file.is_open()) {
        std::string line;
        /* go through all the lines until a match with arr_name is found */
        while(std::getline(file,line)) {
            if(line.find(vec_name)!=std::string::npos) {
                for (size_t i=0; i<no_elem; i++) {
                    std::unordered_set<T>* set=new std::unordered_set<T>;
                    if(std::getline(file,line)) {
                        if (line.compare("x")==0) {
                            continue;
                        } else {
                            std::stringstream line_stream(line);
                            while (line_stream.good()) {
                                T x;
                                line_stream >> x;
                                if (!line_stream.fail()) {
                                    set->insert(x);
                                }
                            }
                            vec.push_back(set);
                        }
                    } else {
                        try {
                            throw std::runtime_error("FileHandler:readVecSet: Unable to read vector.");
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
            throw std::runtime_error("FileHandler:readVecSet: Vector not found.");
        } catch (std::exception &e) {
            std::cout << e.what() << "\n";
            return 0;
        }
    } else {
        try {
            throw std::runtime_error("FileHandler:readVecSet: Unable to open input file.");
        } catch (std::exception &e) {
            std::cout << e.what() << "\n";
            return 0;
        }
    }
}

/*! Read array of vectors (can be thought of as a 2-d table) from file
 * \param[in] filename  Name of the file
 * \param[in] arr             Pointer to the pre-alocated array that will contain the read array value
 * \param[in] no_elem     The size of the array
 * \param[in] array_name     The name of the array whose value is to be read
 * \param[out] out_flag          A flag that is 0 when reading was unsuccessful, and is 1 when reading was successful*/
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
/*! Read array of unordered sets (can be thought of as a 2-d table) from file
 * \param[in] filename  Name of the file
 * \param[in] arr             Pointer to the array that will contain the read array value
 * \param[in] no_elem     The size of the array
 * \param[in] arr_name     The name of the array whose value is to be read
 * \param[out] out_flag          A flag that is 0 when reading was unsuccessful, and is 1 when reading was successful*/
template<class T>
int readArrSet(const std::string& filename, std::unordered_set<T>** arr, size_t no_elem, const std::string& arr_name) {
    std::ifstream file;
    file.open(filename);
    if (file.is_open()) {
        std::string line;
        /* go through all the lines until a match with arr_name is found */
        while(std::getline(file,line)) {
            if(line.find(arr_name)!=std::string::npos) {
                for (size_t i=0; i<no_elem; i++) {
                    if(std::getline(file,line)) {
                        if (line.compare("x")==0) {
                            continue;
                        } else {
                            std::stringstream line_stream(line);
                            while (line_stream.good()) {
                                T x;
                                line_stream >> x;
                                if (!line_stream.fail()) {
                                    arr[i]->insert(x);
                                }
                            }
                        }
                    } else {
                        try {
                            throw std::runtime_error("FileHandler:readArrSet: Unable to read vector.");
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
            throw std::runtime_error("FileHandler:readArrSet: Array not found.");
        } catch (std::exception &e) {
            std::cout << e.what() << "\n";
            return 0;
        }
    } else {
        try {
            throw std::runtime_error("FileHandler:readArrSet: Unable to open input file.");
        } catch (std::exception &e) {
            std::cout << e.what() << "\n";
            return 0;
        }
    }
}
/*! Create a file OR erase previous data written to a file
 * \param[in] filename  The name of the file*/
void create(const std::string& filename) {
    std::ofstream file;
    file.open(filename, std::ofstream::out | std::ofstream::trunc);
    if (file.is_open()) {
        file.close();
    }
}
/* some functions for writing data to file */
/*! Write a member to a file. (A member is an attribute whose value is a scalar.)
 * \param[in] filename  Name of the file
 * \param[in] member_name     The name of the member
 * \param[in] member_value   The value of the member
 * \param[in] mode                     [Optional] The writing mode: "a" for append, "w" for write/overwrite. Default="a".*/
template<class T>
void writeMember(const std::string& filename, const std::string& member_name, T member_value, const char* mode="a") {
    std::ofstream file;
    if (!strcmp(mode,"a")) {
        file.open(filename, std::ios_base::app);
    } else if (!strcmp(mode,"w")) {
        file.open(filename, std::ios_base::out);
    } else {
        try {
            throw std::runtime_error("FileHandler:writeMember: Invalid mode.");
        } catch (std::exception &e) {
            std::cout << e.what() << "\n";
        }
    }

    if (file.is_open()) {
        file << "# " << member_name << "\n";
        file << member_value << "\n";
        file.close();
    } else {
        try {
            throw std::runtime_error("FileHandler:writeMember: Unable to open output file.");
        } catch (std::exception &e) {
            std::cout << e.what() << "\n";
        }
    }
}

/*! Write 1-dimensional integer vector to file
 * \param[in] filename  Name of the file
 * \param[in] vec_name     The name of the vector to be written
 * \param[in] v                     The vector
 * \param[in] mode              [Optional] The writing mode: "a" for append, "w" for write/overwrite. Default="a".*/
template<class T>
void writeVec(const std::string& filename, const std::string& vec_name, std::vector<T>& v, const char* mode="a") {
    std::ofstream file;
    if (!strcmp(mode,"a")) {
        file.open(filename, std::ios_base::app);
    } else if (!strcmp(mode,"w")) {
        file.open(filename, std::ios_base::out);
    } else {
        try {
            throw std::runtime_error("FileHandler:writeVec: Invalid mode.");
        } catch (std::exception &e) {
            std::cout << e.what() << "\n";
        }
    }
    if (file.is_open()) {
        file << "# " << vec_name << "\n";
        for (size_t i=0; i<v.size(); i++) {
            file << v[i] << "\n";
        }
        file.close();
    } else {
        try {
            throw std::runtime_error("FileHandler:writeVec: Unable to open output file.");
        } catch (std::exception &e) {
            std::cout << e.what() << "\n";
        }
    }
}

/*! Write 1-dimensional vector of pointers to file
 * \param[in] filename  Name of the file
 * \param[in] vec_name     The name of the vector
 * \param[in] v                    The vector
 * \param[in] mode             [Optional] The writing mode: "a" for append, "w" for write/overwrite. Default="a".*/
template<class T>
void writeVec(const std::string& filename, const std::string& vec_name, std::vector<T*>& v, const char* mode="a") {
    std::ofstream file;
    if (!strcmp(mode,"a")) {
        file.open(filename, std::ios_base::app);
    } else if (!strcmp(mode,"w")) {
        file.open(filename, std::ios_base::out);
    } else {
        try {
            throw std::runtime_error("FileHandler:writeVec: Invalid mode.");
        } catch (std::exception &e) {
            std::cout << e.what() << "\n";
        }
    }
    if (file.is_open()) {
        file << "# " << vec_name << "\n";
        for (size_t i=0; i<v.size(); i++) {
            file << *v[i] << "\n";
        }
        file.close();
    } else {
        try {
            throw std::runtime_error("FileHandler:writeVec: Unable to open output file.");
        } catch (std::exception &e) {
            std::cout << e.what() << "\n";
        }
    }
}

/*! Write 1-dimensional integer set (unordered) to file
 * \param[in] filename  Name of the file
 * \param[in] set_name     The name of the set
 * \param[in] s                    The set
 * \param[in] mode              [Optional] The writing mode: "a" for append, "w" for write/overwrite. Default="a".*/
template<class T>
void writeSet(const std::string& filename, const std::string& set_name, std::unordered_set<T>& s, const char* mode="a") {
    std::ofstream file;
    if (!strcmp(mode,"a")) {
        file.open(filename, std::ios_base::app);
    } else if (!strcmp(mode,"w")) {
        file.open(filename, std::ios_base::out);
    } else {
        try {
            throw std::runtime_error("FileHandler:writeSet: Invalid mode.");
        } catch (std::exception &e) {
            std::cout << e.what() << "\n";
        }
    }
    if (file.is_open()) {
        file << "# " << set_name << "\n";
        for (auto i=s.begin(); i!=s.end(); ++i) {
            file << *i << "\n";
        }
        file.close();
    } else {
        try {
            throw std::runtime_error("FileHandler:writeSet: Unable to open output file.");
        } catch (std::exception &e) {
            std::cout << e.what() << "\n";
        }
    }
}

/*! Write array of vectors (can be thought of as a 2-d table) from file
 * \param[in] filename  Name of the file
 * \param[in] arr_name     The name of the array
 * \param[in] arr                The array
 * \param[in] mode              [Optional] The writing mode: "a" for append, "w" for write/overwrite. Default="a".*/
template<class T>
void writeArrVec(const std::string& filename, const std::string& arr_name, std::vector<T>** arr, size_t no_elem, const char* mode="a") {
    std::ofstream file;
    if (!strcmp(mode,"a")) {
        file.open(filename, std::ios_base::app);
    } else if (!strcmp(mode,"w")) {
        file.open(filename, std::ios_base::out);
    } else {
        try {
            throw std::runtime_error("FileHandler:writeArrVec: Invalid mode.");
        } catch (std::exception &e) {
            std::cout << e.what() << "\n";
        }
    }
    if (file.is_open()) {
        file << "# " << arr_name << "\n";
        for (size_t i=0; i<no_elem; i++) {
            if (arr[i]->size()==0) {
                file << "x\n";
            } else {
                for (size_t j=0; j<arr[i]->size(); j++) {
                    file << (*arr[i])[j] << " ";
                }
                file << "\n";
            }
        }
        file.close();
    } else {
        try {
            throw std::runtime_error("FileHandler:writeArrVec: Unable to open output file.");
        } catch (std::exception &e) {
            std::cout << e.what() << "\n";
        }
    }
}

/*! Write array of unordered sets (can be thought of as a 2-d table) to file
 * \param[in] filename  Name of the file
 * \param[in] arr_name     The name of the array
 * \param[in] arr                 The array
 * \param[in] mode                [Optional] The writing mode: "a" for append, "w" for write/overwrite. Default="a".*/
template<class T>
void writeArrSet(const std::string& filename, const std::string& arr_name, std::unordered_set<T>** arr, size_t no_elem, const char* mode="a") {
    std::ofstream file;
    if (!strcmp(mode,"a")) {
        file.open(filename, std::ios_base::app);
    } else if (!strcmp(mode,"w")) {
        file.open(filename, std::ios_base::out);
    } else {
        try {
            throw std::runtime_error("FileHandler:writeArrSet: Invalid mode.");
        } catch (std::exception &e) {
            std::cout << e.what() << "\n";
        }
    }
    if (file.is_open()) {
        file << "# " << arr_name << "\n";
        for (size_t i=0; i<no_elem; i++) {
            if (arr[i]->size()==0) {
                file << "x\n";
            } else {
                for (auto it=arr[i]->begin(); it!=arr[i]->end(); it++) {
                    file << (*it) << " ";
                }
                file << "\n";
            }
        }
        file.close();
    } else {
        try {
            throw std::runtime_error("FileHandler:writeArrVec: Unable to open output file.");
        } catch (std::exception &e) {
            std::cout << e.what() << "\n";
        }
    }
}

/*! Write vector of references to unordered sets (can be thought of as a 2-d table) to file
 * \param[in] filename  Name of the file
 * \param[in] vec_name     The name of the vector
 * \param[in] vec                The vector
 * \param[in] mode                     [Optional] The writing mode: "a" for append, "w" for write/overwrite. Default="a".*/
template<class T>
void writeVecSet(const std::string& filename, const std::string& vec_name, std::vector<std::unordered_set<T>*> vec, const char* mode="a") {
    std::ofstream file;
    if (!strcmp(mode,"a")) {
        file.open(filename, std::ios_base::app);
    } else if (!strcmp(mode,"w")) {
        file.open(filename, std::ios_base::out);
    } else {
        try {
            throw std::runtime_error("FileHandler:writeVecSet: Invalid mode.");
        } catch (std::exception &e) {
            std::cout << e.what() << "\n";
        }
    }
    if (file.is_open()) {
        file << "# " << vec_name << "\n";
        for (size_t i=0; i<vec.size(); i++) {
            if (vec[i]->size()==0) {
                file << "x\n";
            } else {
                for (auto it=vec[i]->begin(); it!=vec[i]->end(); it++) {
                    file << (*it) << " ";
                }
                file << "\n";
            }
        }
        file.close();
    } else {
        try {
            throw std::runtime_error("FileHandler:writeVecSet: Unable to open output file.");
        } catch (std::exception &e) {
            std::cout << e.what() << "\n";
        }
    }
}
