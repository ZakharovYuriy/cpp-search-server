#include "remove_duplicates.h"
#include "search_server.h"
#include <map>
#include <set>
#include <string>
#include <iostream>

using namespace std;

void RemoveDuplicates(SearchServer &search_server) {
	map<set<string_view>, int> list_of_words_in_documents;
	set<int> remove_ids;
	for (auto id : search_server) {
		set<string_view> words;
		transform(search_server.GetWordFrequencies(id).begin(),
				search_server.GetWordFrequencies(id).end(),
				inserter(words, words.begin()),
				[](const std::pair<std::string_view, double> &word_freq) {
					return word_freq.first;
				});

		if (list_of_words_in_documents.count(words) == 0) {
			list_of_words_in_documents[words] = id;
		} else {
			remove_ids.insert(id);
		}
	}

	for (int id : remove_ids) {
		search_server.RemoveDocument(id);
		cout << "Found duplicate document id "s << id << endl;
	}
}
