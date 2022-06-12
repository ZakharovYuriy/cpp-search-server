#pragma once
#include "search_server.h"
#include "document.h"
#include <string>
#include <vector>
#include <deque>

class RequestQueue {
public:
    RequestQueue() = default;
    explicit RequestQueue(const SearchServer& search_server) :search_server_{ search_server } {
    }

    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate);
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);
    std::vector<Document> AddFindRequest(const std::string& raw_query);
    int GetNoResultRequests() const;

private:
    struct QueryResult {
        std::string query = "";
        int no_empty_query = 0;
    };
    std::deque<QueryResult> requests_ = {};
    const static int sec_in_day_ = 1440;
    const SearchServer& search_server_;
};

template <typename DocumentPredicate>
std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
    QueryResult q;
    q.query = raw_query;
    q.no_empty_query = search_server_.FindTopDocuments(raw_query, document_predicate).size();
    requests_.push_back(q);
    if (requests_.size() > sec_in_day_) {
        requests_.pop_front();
    }
    return search_server_.FindTopDocuments(raw_query, document_predicate);
}