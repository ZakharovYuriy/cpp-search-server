#pragma once
#include <string>
#include <vector>
#include <iostream>

struct Document {
	Document() = default;
	Document(int id, double relevance, int rating) :
			id(id), relevance(relevance), rating(rating) {
	}
	int id = 0;
	double relevance = 0.0;
	int rating = 0;
};

enum class DocumentStatus {
	ACTUAL, IRRELEVANT, BANNED, REMOVED,
};

void PrintDocument(const Document &document);
void PrintMatchDocumentResult(int document_id,
		const std::vector<std::string_view> &words, DocumentStatus status);

std::ostream& operator<<(std::ostream &out, const Document &document);

