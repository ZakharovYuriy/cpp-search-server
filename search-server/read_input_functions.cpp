#include "read_input_functions.h"
#include "search_server.h"
#include "document.h"
#include <iostream>
#include <string>
#include <vector>

using namespace std;

string ReadLine(){
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result;
    cin >> result;
    ReadLine();
    return result;
}

void MatchDocuments(const SearchServer& search_server, const string& query) {
    using namespace std;
    try {
        cout << "Matching documents on request: "s << query << endl;
        //const int document_count = search_server.GetDocumentCount();
        for (const int document_id : search_server) {
            const auto [words, status] = search_server.MatchDocument(query, document_id);
            PrintMatchDocumentResult(document_id, words, status);
        }
    }
    catch (const invalid_argument& e) {
        cout << "Error in matching documents to a request "s << query << ": "s << e.what() << endl;
    }
}

void FindTopDocuments(const SearchServer& search_server, const string& raw_query) {
    using namespace std;
    cout << "Search results for the query: "s << raw_query << endl;
    try {
        for (const Document& document : search_server.FindTopDocuments(raw_query)) {
            PrintDocument(document);
        }
    }
    catch (const invalid_argument& e) {
        cout << "Search error: "s << e.what() << endl;
    }
}

void AddDocument(SearchServer& search_server, int document_id, const string& document, DocumentStatus status,
    const vector<int>& ratings) {
    using namespace std;
    try {
        search_server.AddDocument(document_id, document, status, ratings);
    }
    catch (const invalid_argument& e) {
        cout << "Error adding a document"s << document_id << ": "s << e.what() << endl;
    }
}
