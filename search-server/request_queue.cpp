#include "request_queue.h"
#include <string>
#include <vector>
#include <algorithm>

std::vector<Document> RequestQueue::AddFindRequest(const std::string &raw_query,
		DocumentStatus status) {
	QueryResult q;
	q.query = raw_query;
	q.no_empty_query =
			search_server_.FindTopDocuments(raw_query, status).size();
	requests_.push_back(q);
	if (requests_.size() > sec_in_day_) {
		requests_.pop_front();
	}
	return search_server_.FindTopDocuments(raw_query, status);
}

std::vector<Document> RequestQueue::AddFindRequest(
		const std::string &raw_query) {
	QueryResult q;
	q.query = raw_query;
	q.no_empty_query = search_server_.FindTopDocuments(raw_query).size();
	requests_.push_back(q);
	if (requests_.size() > sec_in_day_) {
		requests_.pop_front();
	}
	return search_server_.FindTopDocuments(raw_query);
}

int RequestQueue::GetNoResultRequests() const {
	return std::count_if(requests_.begin(), requests_.end(), [](QueryResult i) {
		return i.no_empty_query == 0;
	});
}
