#pragma once
#include <cctype>

namespace utils
{

	// templated version of equality so it could work with both char and wchar_t
	template<typename charType>
	struct ins_equal {
		ins_equal( const std::locale& loc ) : loc_(loc) {}
		bool operator()(charType ch1, charType ch2) {
			return std::toupper(ch1, loc_) == std::toupper(ch2, loc_);
		}
	private:
		const std::locale& loc_;
	};

	// find substring (case insensitive)
	template<typename T>
	int ci_find_substr( const T& str1, const T& str2, const std::locale& loc = std::locale() )
	{
		typename T::const_iterator it = std::search( str1.begin(), str1.end(), 
			str2.begin(), str2.end(), ins_equal<typename T::value_type>(loc) );
		if ( it != str1.end() ) return it - str1.begin();
		else return T::npos; // not found
	}

	// find substring (case insensitive)
	template<typename T>
	bool ci_find_substr_bool( const T& str1, const T& str2, const std::locale& loc = std::locale() )
	{
		typename T::const_iterator it = std::search( str1.begin(), str1.end(), 
			str2.begin(), str2.end(), ins_equal<typename T::value_type>(loc) );
		if ( it != str1.end() )
			return true;
		
		return false; // not found
	}

	bool inline static findStringIC(const std::string & toFind, const std::string & strIn)
	{
		auto it = std::search(
			toFind.begin(), toFind.end(),
			strIn.begin(),   strIn.end(),
			[](char ch1, char ch2) { return std::toupper(ch1) == std::toupper(ch2); }
		);
		if (it != toFind.end() ) return true;
		return false;
	}

}