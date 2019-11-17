/* File: Functions.hpp
 *  Created by: Kaushik
 *  Date: 15/11/2019
 *
 *  Forward declaration of some auxiliary functions */

#ifndef FUNCTIONS_H_INCLUDED
#define FUNCTIONS_H_INCLUDED

/* File I/O */
template<class T> int readMemberFromFile(const std::string& filename, T& member_value, const std::string& member_name);
template<class T>
int readArrayFromFile(const std::string& filename, T& array_value, const std::string& array_name);

#endif
