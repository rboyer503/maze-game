#ifndef UTILITY_H
#define UTILITY_H

#include <sstream>
#include <string>


template<typename T>
bool validateIntRange(const std::string & input, T & output, T min, T max)
{
	// Integer input validation with range checking.
	std::istringstream iss(input);
	if (!(iss >> output))
		return false;

	if ( (output < min) || ( output > max) )
		return false;

	return true;
}

#endif // UTILITY_H
