#pragma once

//#include <iostream>
//#include <fstream>
//#include <map>

#include <cstdio>   // for printf()
#include <iostream> // for std::cin.*()
#include <map>      // for std::map
#include <fstream>  // for std::ifstream
#include <sstream>  // for std::getline()

//#include <cstdlib>  // for exit(), EXIT_FAILURE
//#include <sstream>  // for std::ostringstream, std::istringstream
//#include <typeinfo> // for typeid

// Exits the program and displays the specified error message
void EXIT_Error(const std::string& error)
{
	// Use printf() to print the error message to the console
	printf("%s", error.c_str());

	// Ignore any characters left in the input stream
	std::cin.ignore();

	// Wait for the user to press enter before exiting
	std::cin.get();

	// Exit the program with a failure status code
	exit(EXIT_FAILURE);
}

class Convert
{
public:
	// Converts a value of type T to a string
	template <typename T>
	static std::string T_to_string(T const& val)
	{
		// Create an output string stream
		std::ostringstream ostr;

		// Insert the value into the stream
		ostr << val;

		// Return the resulting string
		return ostr.str();
	}

	// Converts a string to a value of type T
	template <typename T>
	static T string_to_T(std::string const& val)
	{
		// Create an input string stream from the input string
		std::istringstream istr(val);

		// Declare a variable to hold the result
		T returnVal;

		// Try to extract the value from the stream
		if (!(istr >> returnVal))
		{
			// If the extraction fails, exit the program with an error message
			EXIT_Error("[FILE-PARSER] File: Not a valid " + (std::string)typeid(T).name() + " received!\n");
		}

		// Return the extracted value
		return returnVal;
	}

	// Specialization of string_to_T for std::string type
	template <>
	static std::string string_to_T(std::string const& val)
	{
		// Just return the input string
		return val;
	}
};


class FileParser
{
private:
	// Map to store the content of the file, with the key being the name of the parameter
	// and the value being the associated value
	std::map<std::string, std::string> mContent;

	// Name of the file
	std::string sName;

	// Removes the comment (text after the ';' character) from the given line
	void RemoveComment(std::string& line) const
	{
		// Find the position of the ';' character
		size_t pos = line.find(';');

		// If the ';' character was found, erase everything after it
		if (pos != line.npos)
			line.erase(pos);
	}

	// Returns true if the given line is empty or consists only of whitespace characters
	bool OnlyWhitespace(const std::string& line) const
	{
		// Find the first non-whitespace character
		size_t pos = line.find_first_not_of(' ');

		// If no non-whitespace characters were found, the line is empty or consists only of whitespace
		return (pos == line.npos);
	}

	// Returns true if the given line is a valid line (i.e. not a comment and not empty)
	bool ValidLine(const std::string& line) const
	{
		// Make a copy of the line and remove leading whitespace
		std::string temp = line;
		temp.erase(0, temp.find_first_not_of("\t "));

		// If the first non-whitespace character is '=', the line is invalid
		if (temp[0] == '=')
			return false;

		// Find the position of the '=' character
		size_t pos = temp.find('=');

		// If there is no '=' character, the line is invalid
		if (pos == temp.npos)
			return false;

		// Check if there is any non-whitespace character after the '=' character
		for (size_t i = pos + 1; i < temp.length(); i++)
		{
			if (temp[i] != ' ')
				return true; // non-whitespace character found, line is valid
		}

		// No non-whitespace character found after the '=', line is invalid
		return false;
	}


	// Extracts the key (i.e. the text before the '=' character) from the given line and stores it in the 'key' parameter
	void ExtractKey(std::string& key, size_t const& sepPos, const std::string& line) const
	{
		// Extract the key from the line
		key = line.substr(0, sepPos);

		// Find the position of the first tab or space character
		size_t pos = key.find_first_of("\t ");

		// If a tab or space character was found, erase everything after it
		if (pos != key.npos)
			key.erase(pos);
	}

	// Extracts the value (i.e. the text after the '=' character) from the given line and stores it in the 'value' parameter
	void ExtractValue(std::string& value, size_t const& sepPos, const std::string& line) const
	{
		// Extract the value from the line
		value = line.substr(sepPos + 1);

		// Remove leading whitespace
		value.erase(0, value.find_first_not_of("\t "));

		// Remove trailing whitespace
		value.erase(value.find_last_not_of("\t ") + 1);
	}

	// Extracts the key-value pair from the given line and stores it in the map
	void ExtractContent(const std::string& line)
	{
		// Make a copy of the line and remove leading whitespace
		std::string temp = line;
		temp.erase(0, temp.find_first_not_of("\t "));

		// Find the position of the '=' character
		size_t sepPos = temp.find('=');

		// Extract the key and value from the line
		std::string key, value;
		ExtractKey(key, sepPos, temp);
		ExtractValue(value, sepPos, temp);

		// If the key does not already exist in the map, insert the key-value pair
		if (!KeyExists(key))
			mContent.insert(std::pair<std::string, std::string>(key, value));
		else
			EXIT_Error("[FILE-PARSER] File has a duplicate error. Please correct it!\n");
	}

	// Parses the given line and stores the key-value pair in the map
	// lineNo = the current line number in the file
	// line = the current line, with comments removed
	void ParseLine(const std::string& line, size_t const lineNo)
	{
		// Check if the line contains a '=' character
		if (line.find('=') == line.npos)
			EXIT_Error("[FILE-PARSER] File is missing one or more separators. Check line: " + Convert::T_to_string(lineNo) + "\n");

		// Check if the line is a valid line (i.e. not a comment and not empty)
		if (!ValidLine(line))
			EXIT_Error("[FILE-PARSER] File is missing one or more user values. Check line: " + Convert::T_to_string(lineNo) + "\n");

		// Extract the key-value pair from the line and store it in the map
		ExtractContent(line);
	}

	// Extracts the keys and values from the file and stores them in the map
	void ExtractKeys()
	{
		// Open the file
		std::ifstream file;
		file.open(sName.c_str());

		// Check if the file was successfully opened
		if (!file)
			EXIT_Error("[FILE-PARSER] '" + sName + "' was not found in directory.\n");

		// Read each line of the file
		std::string line;
		size_t lineNo = 0;
		while (std::getline(file, line))
		{
			lineNo++; // keep track of the line number

			// Make a copy of the line
			std::string temp = line;

			// Skip empty lines
			if (temp.empty())
				continue;

			// Remove comments from the line
			RemoveComment(temp);

			// Skip lines that contain only whitespace
			if (OnlyWhitespace(temp))
				continue;

			// Parse the line and store the key-value pair in the map
			ParseLine(temp, lineNo);
		}

		// Close the file
		file.close();
	}

public:
	// Constructs a FileParser object and extracts the keys and values from the file
	FileParser(const std::string& sName)
	{
		// Store the name of the file
		this->sName = sName;

		// Extract the keys and values from the file
		ExtractKeys();
	}

	// Returns true if the given key exists in the map
	bool KeyExists(const std::string& key) const
	{
		return mContent.find(key) != mContent.end();
	}

	// Returns the value associated with the given key, or the default value if the key does not exist
	template <typename ValueType>
	ValueType GetValueOfKey(const std::string& key, ValueType const& defaultValue = ValueType()) const
	{
		// If the key does not exist, return the default value
		if (!KeyExists(key))
			return defaultValue;

		// Return the value associated with the key, converted to the specified type
		return Convert::string_to_T<ValueType>(mContent.find(key)->second);
	}
};