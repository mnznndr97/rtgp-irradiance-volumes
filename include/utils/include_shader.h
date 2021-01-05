#pragma once

using namespace std;

#include <std_include.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <utils/SharerShader.hpp>


class Include {
private:
	typedef std::unordered_map<std::string, SharedShader*> Map;
	typedef Map::iterator Iterator;
	/// <summary>
	/// Files include map. For the moment we don't care about
	/// </summary>
	Map _includeMapping;
public:
	static Include Instance;

	const SharedShader* operator[](const std::string& file)
	{
		Iterator findResult = _includeMapping.find(file);
		if (findResult != _includeMapping.end()) {
			return findResult->second;
		}
		else
		{
			SharedShader* newShader = new SharedShader(file);
			_includeMapping.insert(std::pair(file, newShader));
			return newShader;
		}
	}

	void Clear() {
		for (std::pair<const std::string, SharedShader*>& it : _includeMapping) {
			delete it.second;
		}
		_includeMapping.clear();
	}
};

// Singleton definition
Include Include::Instance;