#pragma once

#include <vector>
#include <string>
#include <set>

template<typename StringType>
std::vector<std::string_view> SplitIntoWords(const StringType &str);

template<typename StringContainer>
std::set<std::string> MakeUniqueNonEmptyStrings(const StringContainer &strings);

template<typename StringContainer>
std::set<std::string> MakeUniqueNonEmptyStrings(const StringContainer &strings);

template<typename StringType>
std::vector<std::string_view> SplitIntoWords(const StringType &str) {
	using namespace std;
	if constexpr (std::is_same_v<std::decay_t<StringType>,
			std::decay_t<std::string>>) {
		return SplitIntoWords(static_cast<string_view>(str));
	} else {
		vector < string_view > result;
		int64_t pos = 0;
		const int64_t pos_end = str.npos;
		while (true) {
			int64_t space = str.find(' ', pos);
			result.push_back(
					space == pos_end ?
							str.substr(pos) : str.substr(pos, space - pos));
			if (space == pos_end) {
				break;
			} else {
				pos = space + 1;
			}
		}
		return result;
	}
}
;

template<typename StringContainer>
std::set<std::string> MakeUniqueNonEmptyStrings(
		const StringContainer &strings) {
	using namespace std;
	set < string > non_empty_strings;
	for (const auto &str : strings) {
		if (!str.empty()) {
			non_empty_strings.insert(static_cast<string>(str));
		}
	}
	return non_empty_strings;
}
