#include "document.h"
#include <string>
#include <vector>
#include <iostream>

void PrintDocument(const Document &document) {
	using namespace std;
	cout << "{ "s << "document_id = "s << document.id << ", "s
			<< "relevance = "s << document.relevance << ", "s << "rating = "s
			<< document.rating << " }"s << endl;
}

void PrintMatchDocumentResult(int document_id,
		const std::vector<std::string_view> &words, DocumentStatus status) {
	using namespace std;
	cout << "{ "s << "document_id = "s << document_id << ", "s << "status = "s
			<< static_cast<int>(status) << ", "s << "words ="s;
	for (string_view word : words) {
		cout << ' ' << word;
	}
	cout << "}"s << endl;
}

std::ostream& operator<<(std::ostream &out, const Document &document) {
	using namespace std;
	out << "{ "s << "document_id = "s << document.id << ", "s << "relevance = "s
			<< document.relevance << ", "s << "rating = "s << document.rating
			<< " }"s;
	return out;
}
