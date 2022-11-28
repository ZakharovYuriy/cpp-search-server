#pragma once

#include "read_input_functions.h"
#include "request_queue.h"
#include "paginator.h"
#include "search_server.h"
#include "document.h"

#include "process_queries.h"
#include "log_duration.h"
#include <random>
#include <execution>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <tuple>

template<typename Function>
void RunTestImpl(const Function &Func, const std::string &func_name) {
	Func();
	std::cerr << func_name << " OK" << std::endl;
}

#define RUN_TEST(func)  RunTestImpl(func,#func)

template<typename T, typename U>
void AssertEqualImpl(const T &t, const U &u, const std::string &t_str,
		const std::string &u_str, const std::string &file,
		const std::string &func, unsigned line, const std::string &hint) {
	using namespace std;
	if (t != u) {
		cout << boolalpha;
		cout << file << "("s << line << "): "s << func << ": "s;
		cout << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
		cout << t << " != "s << u << "."s;
		if (!hint.empty()) {
			cout << " Hint: "s << hint;
		}
		cout << endl;
		abort();
	}
}

#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

void AssertImpl(bool value, const std::string &expr_str,
		const std::string &file, const std::string &func, unsigned line,
		const std::string &hint) {
	using namespace std;
	if (!value) {
		cout << file << "("s << line << "): "s << func << ": "s;
		cout << "ASSERT("s << expr_str << ") failed."s;
		if (!hint.empty()) {
			cout << " Hint: "s << hint;
		}
		cout << endl;
		abort();
	}
}

#define ASSERT(expr) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_HINT(expr, hint) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, hint)

//Adding documents. The added document must be located by a search query that contains words from the document
void TestDocumentAdd() {
	using namespace std;
//when document1
	const int kAddedDocumentId = 42;
	const string kContent = "cat in the city"s;
	const vector<int> kRatings = { 1, 2, 3 };

	{
		SearchServer server(""s);
		server.AddDocument(kAddedDocumentId, kContent, DocumentStatus::ACTUAL,
				kRatings);

		const auto kFoundDocs = server.FindTopDocuments("in"s);
		ASSERT_EQUAL(kFoundDocs.size(), 1u);
		const Document &kFoundDocument0 = kFoundDocs[0];
		ASSERT_EQUAL(kFoundDocument0.id, kAddedDocumentId);
	}
	//when there are several documents and the words do not match
	{
		SearchServer server(""s);
		server.AddDocument(kAddedDocumentId, kContent, DocumentStatus::ACTUAL,
				kRatings);

		const int kSecondAddedDocumentId = 3;
		const string kSecondContent = "dog on box"s;
		const vector<int> kSecondRatings = { 4, 5, 6 };
		server.AddDocument(kSecondAddedDocumentId, kSecondContent,
				DocumentStatus::ACTUAL, kSecondRatings);

		const auto kFirsFoundDocs = server.FindTopDocuments("in"s);
		ASSERT_EQUAL(kFirsFoundDocs.size(), 1u);
		const Document &kFirstFoundDocument = kFirsFoundDocs[0];
		ASSERT_EQUAL(kFirstFoundDocument.id, kAddedDocumentId);

		const auto kSecondFoundDocs = server.FindTopDocuments("on"s);
		ASSERT_EQUAL(kSecondFoundDocs.size(), 1u);
		const Document &kSecondFoundDocument = kSecondFoundDocs[0];
		ASSERT_EQUAL(kSecondFoundDocument.id, kSecondAddedDocumentId);
	}
	//when there are several documents and the words in the documents match
	{
		SearchServer server(""s);
		server.AddDocument(kAddedDocumentId, kContent, DocumentStatus::ACTUAL,
				kRatings);

		const int kSecondAddedDocumentId = 3;
		const string kSecondContent = "dog in the box"s;
		const vector<int> kSecondRatings = { 4, 5, 6 };
		server.AddDocument(kSecondAddedDocumentId, kSecondContent,
				DocumentStatus::ACTUAL, kSecondRatings);

		const auto kFirstFoundDocs = server.FindTopDocuments("in"s);
		ASSERT_EQUAL(kFirstFoundDocs.size(), 2u);
		const Document &kFirstFoundDocument = kFirstFoundDocs[0];
		const Document &kSecondFoundDocument = kFirstFoundDocs[1];
		ASSERT_EQUAL(kFirstFoundDocument.id, kSecondAddedDocumentId);
		ASSERT_EQUAL(kSecondFoundDocument.id, kAddedDocumentId);

		const auto kSecondFoundDocs = server.FindTopDocuments("dog"s);
		ASSERT_EQUAL(kSecondFoundDocs.size(), 1u);
		const Document &kThirdFoundDocs = kSecondFoundDocs[0];
		ASSERT_EQUAL(kThirdFoundDocs.id, kSecondAddedDocumentId);
	}
}

// The test checks that the search engine excludes stop words when adding documents
void TestExcludeStopWordsFromAddedDocumentContent() {
	using namespace std;
	const int kDocId = 42;
	const string kContent = "cat in the city"s;
	const vector<int> kRatings = { 1, 2, 3 };
	{
		SearchServer server(""s);
		server.AddDocument(kDocId, kContent, DocumentStatus::ACTUAL, kRatings);
		const auto kFoundDocs = server.FindTopDocuments("in"s);
		ASSERT_EQUAL(kFoundDocs.size(), 1u);
		const Document &kFoundDocument = kFoundDocs[0];
		ASSERT_EQUAL(kFoundDocument.id, kDocId);
	}

	{
		SearchServer server("in the"s);
		server.AddDocument(kDocId, kContent, DocumentStatus::ACTUAL, kRatings);
		ASSERT_HINT(server.FindTopDocuments("in"s).empty(),
				"Stop words must be excluded from documents"s);
	}
}
//Support for negative keywords.
//Documents containing negative keywords of the search query
//should not be included in the search results.
void TestQueryWithMinusWords() {
	using namespace std;
	const int kDocId = 42;
	const string kContent = "cat in the city"s;
	const vector<int> kRatings = { 1, 2, 3 };
	{
		SearchServer server(""s);
		server.AddDocument(kDocId, kContent, DocumentStatus::ACTUAL, kRatings);
		const auto kFoundDocs = server.FindTopDocuments("in"s);
		ASSERT_EQUAL(kFoundDocs.size(), 1u);
		const Document &kFoundDocument = kFoundDocs[0];
		ASSERT_EQUAL(kFoundDocument.id, kDocId);
	}

	{
		SearchServer server(""s);
		server.AddDocument(kDocId, kContent, DocumentStatus::ACTUAL, kRatings);
		ASSERT_HINT(server.FindTopDocuments("-in the"s).empty(),
				"Documents containing negative keywords of the search query should not be included in the search results"s);
	}
}

//Document matching. When matching a document by a search query,
//all the words from the search query present in the document must be returned.
//If there is a match for at least one negative word,
//an empty list of words should be returned.
void TestMatching() {
	using namespace std;
	const int kDocId = 42;
	const string kContent = "cat in the city"s;
	const vector<int> kRatings = { 1, 2, 3 };
	//нет минус слов
	{
		SearchServer server(""s);
		server.AddDocument(kDocId, kContent, DocumentStatus::ACTUAL, kRatings);
		vector<string> ref_vect = { "cat"s, "city"s, "in"s, "the"s };
		vector<string_view> vect_sv(ref_vect.begin(), ref_vect.end());
		tuple<vector<string_view>, DocumentStatus> reference_tuple = { vect_sv,
				DocumentStatus::ACTUAL };
		vector<string_view> b = (get<0>(
				server.MatchDocument("cat in the city"s, 42)));
		ASSERT(server.MatchDocument("cat in the city"s, 42) == reference_tuple);
	}

	{
		SearchServer search_server(""s);
		search_server.AddDocument(2, "белый кот и модный ошейник"s,
				DocumentStatus::ACTUAL, { 8, -3 });
		search_server.AddDocument(3, "белый и модный"s, DocumentStatus::BANNED,
				{ 8, -3 });

		vector<string> ref_vect = { "модный"s };
		vector<string_view> vect_sv(ref_vect.begin(), ref_vect.end());
		vector<string_view> matched_words;
		tuple<vector<string_view>, DocumentStatus> reference_tuple1 = {
				matched_words, DocumentStatus::ACTUAL };
		tuple<vector<string_view>, DocumentStatus> reference_tuple2 = { vect_sv,
				DocumentStatus::BANNED };
		ASSERT_HINT(
				search_server.MatchDocument("-кот модный"s, 2)
						== reference_tuple1,
				"If there are negative keywords, the list should be empty"s);
		ASSERT(
				search_server.MatchDocument("-кот модный"s, 3)
						== reference_tuple2);
	}
}

//Sorting of found documents by relevance.
//The results returned when searching for documents should be sorted
//in descending order of relevance.
void TestSortDescendingRelevance() {
	using namespace std;
	SearchServer search_server("и в на"s);
	search_server.AddDocument(0, "белый кот и модный ошейник"s,
			DocumentStatus::ACTUAL, { 8, -3 });
	search_server.AddDocument(1, "пушистый кот пушистый хвост"s,
			DocumentStatus::ACTUAL, { 7, 2, 7 });
	search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s,
			DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
	search_server.AddDocument(3, "ухоженный скворец евгений"s,
			DocumentStatus::BANNED, { 9 });

	vector<int> id_documents_sorted_by_descending_relevance;
	for (const Document &kDocument : search_server.FindTopDocuments(
			"пушистый ухоженный кот"s)) {
		id_documents_sorted_by_descending_relevance.push_back(kDocument.id);
	}
	vector<int> reference_id_vector = { 1, 0, 2 };
	ASSERT(id_documents_sorted_by_descending_relevance == reference_id_vector);
}

//Calculation of the document rating.
// The rating of the added document is equal to the arithmetic mean of the document ratings.
void TestCalculationRating() {
	using namespace std;
	SearchServer server(""s);
	server.AddDocument(2, "ухоженный пёс выразительные глаза"s,
			DocumentStatus::ACTUAL, { 5, -12, 2, 1 });

	const auto kFoundDocs = server.FindTopDocuments("пёс"s);
	const Document &kFoundDocument = kFoundDocs[0];
	const int kReferenceRating = (5 - 12 + 2 + 1) / 4;
	ASSERT_EQUAL(kFoundDocument.rating, kReferenceRating);
}

//Filtering search results using a predicate set by the user.
void TestSortWithPredictate() {
	using namespace std;
	SearchServer search_server("и в на"s);
	search_server.AddDocument(0, "белый кот и модный ошейник"s,
			DocumentStatus::ACTUAL, { 8, -3 });
	search_server.AddDocument(1, "пушистый кот пушистый хвост"s,
			DocumentStatus::ACTUAL, { 7, 2, 7 });
	search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s,
			DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
	search_server.AddDocument(3, "ухоженный скворец евгений"s,
			DocumentStatus::BANNED, { 9 });

	{
		const auto kFoundDocs = search_server.FindTopDocuments("хвост"s);
		const int kDocId = 1u;
		ASSERT_EQUAL(kFoundDocs.size(), 1u);
		const Document &kFoundDocument = kFoundDocs[0];
		ASSERT_EQUAL(kFoundDocument.id, kDocId);
	}

	vector<int> id_documents_sorted_by_descending_relevance_with_predictate;
	for (const Document &kDocument : search_server.FindTopDocuments(
			"пушистый ухоженный кот"s,
			[](int document_id, DocumentStatus status, int rating) {
				return document_id % 2 == 0;
			})) {
		id_documents_sorted_by_descending_relevance_with_predictate.push_back(
				kDocument.id);
	}
	ASSERT_EQUAL(
			id_documents_sorted_by_descending_relevance_with_predictate.size(),
			2u);
	vector<int> reference_id_vector = { 0, 2 };
	ASSERT(
			id_documents_sorted_by_descending_relevance_with_predictate
					== reference_id_vector);
}
//Search for documents with the specified status.
void TestFindDocumentsWithStatus() {
	using namespace std;
	SearchServer search_server(""s);
	vector<int> reference_id_vector = { };
	search_server.AddDocument(0, "белый кот и модный ошейник"s,
			DocumentStatus::ACTUAL, { 8, -3 });
	search_server.AddDocument(6, "белый кот и модный ошейник"s,
			DocumentStatus::REMOVED, { 8, -3 });
	search_server.AddDocument(8, "белый кот и модный ошейник"s,
			DocumentStatus::IRRELEVANT, { 8, -3 });

	{
		vector<int> result_id_vector;
		for (const Document &kDocument : search_server.FindTopDocuments(
				"пушистый ухоженный кот"s, DocumentStatus::ACTUAL)) {
			result_id_vector.push_back(kDocument.id);
		}
		reference_id_vector = { 0 };
		ASSERT(result_id_vector == reference_id_vector);
	}

	{
		vector<int> result_id_vector;
		for (const Document &kDocument : search_server.FindTopDocuments(
				"пушистый ухоженный кот"s, DocumentStatus::BANNED)) {
			result_id_vector.push_back(kDocument.id);
		}
		reference_id_vector = { };
		ASSERT(result_id_vector.empty());
	}

	{
		vector<int> result_id_vector;
		for (const Document &kDocument : search_server.FindTopDocuments(
				"пушистый ухоженный кот"s, DocumentStatus::REMOVED)) {
			result_id_vector.push_back(kDocument.id);
		}
		reference_id_vector = { 6 };
		ASSERT(result_id_vector == reference_id_vector);
	}

	{
		vector<int> result_id_vector;
		for (const Document &kDocument : search_server.FindTopDocuments(
				"пушистый ухоженный кот"s, DocumentStatus::IRRELEVANT)) {
			result_id_vector.push_back(kDocument.id);
		}
		reference_id_vector = { 8 };
		ASSERT(result_id_vector == reference_id_vector);
	}

}

//Correct calculation of the relevance of the found documents.
void TestRelevancCalculation() {
	using namespace std;
	SearchServer search_server(""s);
	search_server.AddDocument(0, "белый кот модный ошейник"s,
			DocumentStatus::ACTUAL, { 8, -3 });
	search_server.AddDocument(1, "пушистый кот пушистый хвост"s,
			DocumentStatus::ACTUAL, { 7, 2, 7 });
	search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s,
			DocumentStatus::ACTUAL, { 5, -12, 2, 1 });

	const vector<string> &query = { "пушистый"s, "ухоженный"s, "кот"s };

	for (const Document &kFoundDocument : search_server.FindTopDocuments(
			"пушистый ухоженный кот"s)) {
		switch (kFoundDocument.id) {
		case (0):
			ASSERT_EQUAL(kFoundDocument.relevance,
					log(search_server.GetDocumentCount() * 1.0 / 2)
							* (1.0 / 4));
			break;
		case (1):
			ASSERT_EQUAL(kFoundDocument.relevance,
					(log(search_server.GetDocumentCount() * 1.0 / 1) * (2.0 / 4)
							+ log(search_server.GetDocumentCount() * 1.0 / 2)
									* (1.0 / 4)));
			break;
		case (2):
			ASSERT_EQUAL(kFoundDocument.relevance,
					log(search_server.GetDocumentCount() * 1.0 / 1)
							* (1.0 / 4));
			break;
		default:
			ASSERT_HINT(false, "No document found"s);
			break;
		}
	}
}

// The TestSearchServer function is the entry point for running tests
void TestSearchServer() {
	RUN_TEST(TestDocumentAdd);
	RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
	RUN_TEST(TestQueryWithMinusWords);
	RUN_TEST(TestMatching);
	RUN_TEST(TestSortDescendingRelevance);
	RUN_TEST(TestCalculationRating);
	RUN_TEST(TestSortWithPredictate);
	RUN_TEST(TestFindDocumentsWithStatus);
	RUN_TEST(TestRelevancCalculation);
}

//4 sprint
void ManyRequests() {
	using namespace std;

	SearchServer search_server("and in at"s);
	RequestQueue request_queue(search_server);

	search_server.AddDocument(1, "curly cat curly tail"s,
			DocumentStatus::ACTUAL, { 7, 2, 7 });
	search_server.AddDocument(2, "curly dog and fancy collar"s,
			DocumentStatus::ACTUAL, { 1, 2, 3 });
	search_server.AddDocument(3, "big cat fancy collar "s,
			DocumentStatus::ACTUAL, { 1, 2, 8 });
	search_server.AddDocument(4, "big dog sparrow Eugene"s,
			DocumentStatus::ACTUAL, { 1, 3, 2 });
	search_server.AddDocument(5, "big dog sparrow Vasiliy"s,
			DocumentStatus::ACTUAL, { 1, 1, 1 });

	// 1439 requests with zero result
	for (int i = 0; i < 1439; ++i) {
		request_queue.AddFindRequest("empty request"s);
	}
	// still 1439 requests with zero result
	request_queue.AddFindRequest("curly dog"s);
	// new day, first request deleted, 1438 requests with zero result
	request_queue.AddFindRequest("big collar"s);
	// first request deleted, 1437 requests with zero result
	request_queue.AddFindRequest("sparrow"s);
	cout << "Total empty requests: "s << request_queue.GetNoResultRequests()
			<< endl;
}
//4 sprint
void TestPaginate() {
	using namespace std;
	SearchServer search_server("and with"s);

	search_server.AddDocument(1, "funny pet and nasty rat"s,
			DocumentStatus::ACTUAL, { 7, 2, 7 });
	search_server.AddDocument(2, "funny pet with curly hair"s,
			DocumentStatus::ACTUAL, { 1, 2, 3 });
	search_server.AddDocument(3, "big cat nasty hair"s, DocumentStatus::ACTUAL,
			{ 1, 2, 8 });
	search_server.AddDocument(4, "big dog cat Vladislav"s,
			DocumentStatus::ACTUAL, { 1, 3, 2 });
	search_server.AddDocument(5, "big dog hamster Borya"s,
			DocumentStatus::ACTUAL, { 1, 1, 1 });

	const auto search_results = search_server.FindTopDocuments("curly dog"s);
	int page_size = 2;
	const auto pages = Paginate(search_results, page_size);

	// Output the found documents by pages
	for (auto page = pages.begin(); page != pages.end(); ++page) {
		cout << *page << endl;
		cout << "Page break"s << endl;
	}
}

void SpecializationOfTemplates() {
	using namespace std;
	SearchServer search_server("è â íà"s);

	search_server.AddDocument(0, "áåëûé êîò ìîäíûé îøåéíèê"s,
			DocumentStatus::ACTUAL, { 8, -3 });
	search_server.AddDocument(1, "ïóøèñòûé êîò ïóøèñòûé õâîñò"s,
			DocumentStatus::ACTUAL, { 7, 2, 7 });
	search_server.AddDocument(2, "óõîæåííûé ï¸ñ âûðàçèòåëüíûå ãëàçà"s,
			DocumentStatus::ACTUAL, { 5, -12, 2, 1 });

	cout << "ACTUAL by default:"s << endl;
	for (const Document &document : search_server.FindTopDocuments(
			"ïóøèñòûé óõîæåííûé êîò"s)) {
		PrintDocument(document);
	}

	cout << "BANNED:"s << endl;
	for (const Document &document : search_server.FindTopDocuments(
			"ïóøèñòûé óõîæåííûé êîò"s, DocumentStatus::BANNED)) {
		PrintDocument(document);
	}

	cout << "Even ids:"s << endl;
	for (const Document &document : search_server.FindTopDocuments(
			"ïóøèñòûé óõîæåííûé êîò"s,
			[](int document_id, DocumentStatus status, int rating) {
				return document_id % 2 == 0;
			})) {
		PrintDocument(document);
	}

}
void TestRemoveDuplicates() {
	using namespace std;

	SearchServer search_server("and with"s);

	AddDocument(search_server, 1, "funny pet and nasty rat"s,
			DocumentStatus::ACTUAL, { 7, 2, 7 });
	AddDocument(search_server, 2, "funny pet with curly hair"s,
			DocumentStatus::ACTUAL, { 1, 2 });

	//duplicate document 2, will be deleted
	AddDocument(search_server, 3, "funny pet with curly hair"s,
			DocumentStatus::ACTUAL, { 1, 2 });

	//the difference is only in stop words, we consider it a duplicate
	AddDocument(search_server, 4, "funny pet and curly hair"s,
			DocumentStatus::ACTUAL, { 1, 2 });

	// the set of words is the same, we consider a duplicate of document 1
	AddDocument(search_server, 5, "funny funny pet and nasty nasty rat"s,
			DocumentStatus::ACTUAL, { 1, 2 });

	//new words have been added, the duplicate is not
	AddDocument(search_server, 6, "funny pet and not very nasty rat"s,
			DocumentStatus::ACTUAL, { 1, 2 });

	//the set of words is the same as in id 6, despite the different order, we consider a duplicate
	AddDocument(search_server, 7, "very nasty rat and not very funny pet"s,
			DocumentStatus::ACTUAL, { 1, 2 });

	// there are not all words, is not a duplicate
	AddDocument(search_server, 8, "pet with rat and rat and rat"s,
			DocumentStatus::ACTUAL, { 1, 2 });

	// words from different documents, not a duplicate
	AddDocument(search_server, 9, "nasty rat with curly hair"s,
			DocumentStatus::ACTUAL, { 1, 2 });

	cout << "Before duplicates removed: "s << search_server.GetDocumentCount()
			<< endl;
	//RemoveDuplicates(search_server);
	cout << "After duplicates removed: "s << search_server.GetDocumentCount()
			<< endl;
}

//the ProcessQueries function,
//parallelizing the processing of multiple queries to the search engine.
void SimpleTestProcessQueries() {
	using namespace std;
	SearchServer search_server("and with"s);
	int id = 0;
	for (const string &text : { "funny pet and nasty rat"s,
			"funny pet with curly hair"s, "funny pet and not very nasty rat"s,
			"pet with rat and rat and rat"s, "nasty rat with curly hair"s, }) {
		search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, { 1, 2 });
	}

	const vector<string> queries = { "nasty rat -not"s,
			"not very funny nasty pet"s, "curly hair"s };
	id = 0;
	for (const auto &documents : ProcessQueries(search_server, queries)) {
		cout << documents.size() << " documents for query ["s << queries[id++]
				<< "]"s << endl;
	}
}

std::string GenerateWord(std::mt19937 &generator, int max_length) {
	using namespace std;
	const int length = uniform_int_distribution(1, max_length)(generator);
	string word;
	word.reserve(length);
	for (int i = 0; i < length; ++i) {
		word.push_back(uniform_int_distribution(97, 122)(generator));
	}
	return word;
}

std::vector<std::string> GenerateDictionary(std::mt19937 &generator,
		int word_count, int max_length) {
	using namespace std;
	vector<string> words;
	words.reserve(word_count);
	for (int i = 0; i < word_count; ++i) {
		words.push_back(GenerateWord(generator, max_length));
	}
	sort(words.begin(), words.end());
	words.erase(unique(words.begin(), words.end()), words.end());
	return words;
}

std::string GenerateQuery(std::mt19937 &generator,
		const std::vector<std::string> &dictionary, int max_word_count) {
	using namespace std;
	const int word_count = uniform_int_distribution(1, max_word_count)(
			generator);
	string query;
	for (int i = 0; i < word_count; ++i) {
		if (!query.empty()) {
			query.push_back(' ');
		}
		query += dictionary[uniform_int_distribution<int>(0,
				dictionary.size() - 1)(generator)];
	}
	return query;
}

std::vector<std::string> GenerateQueries(std::mt19937 &generator,
		const std::vector<std::string> &dictionary, int query_count,
		int max_word_count) {
	using namespace std;
	vector<string> queries;
	queries.reserve(query_count);
	for (int i = 0; i < query_count; ++i) {
		queries.push_back(GenerateQuery(generator, dictionary, max_word_count));
	}
	return queries;
}

template<typename QueriesProcessor>
void Test(std::string_view mark, QueriesProcessor processor,
		const SearchServer &search_server,
		const std::vector<std::string> &queries) {
	using namespace std;
	LOG_DURATION(mark);
	const auto documents_lists = processor(search_server, queries);
}

#define TEST(processor) Test(#processor, processor, search_server, queries)

void StressTestProcessQueries() {
	using namespace std;
	mt19937 generator;
	const auto dictionary = GenerateDictionary(generator, 10000, 25);
	const auto documents = GenerateQueries(generator, dictionary, 100'000, 10);

	SearchServer search_server(dictionary[0]);
	for (size_t i = 0; i < documents.size(); ++i) {
		search_server.AddDocument(i, documents[i], DocumentStatus::ACTUAL, { 1,
				2, 3 });
	}

	const auto queries = GenerateQueries(generator, dictionary, 10'000, 7);
	TEST(ProcessQueries);
}

void SimpleTestProcessQueriesJoined() {
	using namespace std;

	SearchServer search_server("and with"s);

	int id = 0;
	for (const string &text : { "funny pet and nasty rat"s,
			"funny pet with curly hair"s, "funny pet and not very nasty rat"s,
			"pet with rat and rat and rat"s, "nasty rat with curly hair"s, }) {
		search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, { 1, 2 });
	}

	const vector<string> queries = { "nasty rat -not"s,
			"not very funny nasty pet"s, "curly hair"s };
	for (const Document &document : ProcessQueriesJoined(search_server, queries)) {
		cout << "Document "s << document.id << " matched with relevance "s
				<< document.relevance << endl;
	}
}

//The ProcessQueriesJoined function should,
//like the ProcessQueries function, parallelize the processing
//of several queries to the search engine,
//but return a set of documents in a flat form.
void StressTestProcessQueriesJoined() {
	using namespace std;
	mt19937 generator;
	const auto dictionary = GenerateDictionary(generator, 10000, 25);
	const auto documents = GenerateQueries(generator, dictionary, 100'000, 10);

	SearchServer search_server(dictionary[0]);
	for (size_t i = 0; i < documents.size(); ++i) {
		search_server.AddDocument(i, documents[i], DocumentStatus::ACTUAL, { 1,
				2, 3 });
	}

	const auto queries = GenerateQueries(generator, dictionary, 10'000, 7);
	TEST(ProcessQueriesJoined);
}
//checking the multithreaded version of the RemoveDocument method
//in addition to the single-threaded one.
void SimpleTestRemoveDocument() {
	using namespace std;
	SearchServer search_server("and with"s);

	int id = 0;
	for (const string &text : { "funny pet and nasty rat"s,
			"funny pet with curly hair"s, "funny pet and not very nasty rat"s,
			"pet with rat and rat and rat"s, "nasty rat with curly hair"s, }) {
		search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, { 1, 2 });
	}

	const string query = "curly and funny"s;

	auto report = [&search_server, &query] {
		cout << search_server.GetDocumentCount() << " documents total, "s
				<< search_server.FindTopDocuments(query).size()
				<< " documents for query ["s << query << "]"s << endl;
	};

	report();
	// single-threaded version
	search_server.RemoveDocument(5);
	report();
	// single-threaded version
	search_server.RemoveDocument(execution::seq, 1);
	report();
	// multithreaded version
	search_server.RemoveDocument(execution::par, 2);
	report();
}

template<typename ExecutionPolicy>
void Test(std::string_view mark, SearchServer search_server,
		ExecutionPolicy &&policy) {
	using namespace std;
	LOG_DURATION(mark);
	const int document_count = search_server.GetDocumentCount();
	for (int id = 0; id < document_count; ++id) {
		search_server.RemoveDocument(policy, id);
	}
	cout << search_server.GetDocumentCount() << endl;
}
#define TEST1(mode) Test(#mode, search_server, execution::mode)

void StressTestRemoveDocument() {
	using namespace std;
	mt19937 generator;

	const auto dictionary = GenerateDictionary(generator, 10000, 25);
	const auto documents = GenerateQueries(generator, dictionary, 10'000, 100);

	SearchServer search_server(dictionary[0]);
	for (size_t i = 0; i < documents.size(); ++i) {
		search_server.AddDocument(i, documents[i], DocumentStatus::ACTUAL, { 1,
				2, 3 });
	}
	TEST1(par);
	TEST1(seq);

}

void TestDifferentVersionsMatchDocument() {
	using namespace std;
	SearchServer search_server("and with"s);

	int id = 0;
	for (const string &text : { "funny pet and nasty rat"s,
			"funny pet with curly hair"s, "funny pet and not very nasty rat"s,
			"pet with rat and rat and rat"s, "nasty rat with curly hair"s, }) {
		search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, { 1, 2 });
	}

	const string query = "curly and funny -not"s;

	{
		const auto [words, status] = search_server.MatchDocument(query, 1);
		cout << words.size() << " words for document 1"s << endl;
		// 1 words for document 1
	}

	{
		const auto [words, status] = search_server.MatchDocument(execution::seq,
				query, 2);
		cout << words.size() << " words for document 2"s << endl;
		// 2 words for document 2
	}

	{
		const auto [words, status] = search_server.MatchDocument(execution::par,
				query, 3);
		cout << words.size() << " words for document 3"s << endl;
		// 0 words for document 3
	}
}

void TestDifferentVersionsFindTopDocuments() {
	using namespace std;

	SearchServer search_server("and with"s);

	int id = 0;
	for (const string &text : { "white cat and yellow hat"s,
			"curly cat curly tail"s, "nasty dog with big eyes"s,
			"nasty pigeon john"s, }) {
		search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, { 1, 2 });
	}

	cout << "ACTUAL by default:"s << endl;
	//single-threaded version
	for (const Document &document : search_server.FindTopDocuments(
			"curly nasty cat"s)) {
		PrintDocument(document);
	}
	cout << "BANNED:"s << endl;
	//single-threaded version
	for (const Document &document : search_server.FindTopDocuments(
			execution::seq, "curly nasty cat"s, DocumentStatus::BANNED)) {
		PrintDocument(document);
	}

	cout << "Even ids:"s << endl;
	// multithreaded version
	for (const Document &document : search_server.FindTopDocuments(
			execution::par, "curly nasty cat"s,
			[](int document_id, DocumentStatus status, int rating) {
				return document_id % 2 == 0;
			})) {
		PrintDocument(document);
	}
}

template<typename ExecutionPolicy>
void Test5(std::string_view mark, const SearchServer &search_server,
		const std::vector<std::string> &queries, ExecutionPolicy &&policy) {
	using namespace std;
	LOG_DURATION(mark);
	double total_relevance = 0;
	for (const string_view query : queries) {
		for (const auto &document : search_server.FindTopDocuments(policy,
				query)) {
			total_relevance += document.relevance;
		}
	}
	cout << total_relevance << endl;
}
#define TEST5(policy) Test5(#policy, search_server, queries, execution::policy)

void StressTestDifferentVersionsFindTopDocuments() {
	using namespace std;

	mt19937 generator;

	const auto dictionary = GenerateDictionary(generator, 1000, 10);
	const auto documents = GenerateQueries(generator, dictionary, 10'000, 70);

	SearchServer search_server(dictionary[0]);
	for (size_t i = 0; i < documents.size(); ++i) {
		search_server.AddDocument(i, documents[i], DocumentStatus::ACTUAL, { 1,
				2, 3 });
	}

	const auto queries = GenerateQueries(generator, dictionary, 100, 70);

	TEST5(seq);
	TEST5(par);
}
