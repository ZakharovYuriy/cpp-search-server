#include "search_server.h"
#include "document.h"
#include "string_processing.h"
#include <string>
#include <vector>
#include <execution>
#include "log_duration.h"
#include "list"
#include <tuple>
#include <iostream>


using namespace std;

//PUBLIC METODS
void SearchServer::AddDocument(int document_id, string_view document, DocumentStatus status, const vector<int>& ratings) {
    if ((document_id < 0) || (documents_.count(document_id) > 0)) {
        throw invalid_argument("Invalid document_id"s);
    }

    const auto words = SplitIntoWordsNoStop(document);

    const double inv_word_count = 1.0 / words.size();
    for (const string& word : words) {
        all_words_.insert(word);
        word_to_document_freqs_[*all_words_.find(word)][document_id] += inv_word_count;
        words_frequency_in_document_[document_id][*all_words_.find(word)] += inv_word_count;
    }
    documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });
    document_ids_.insert(document_id);
}

vector<Document> SearchServer::FindTopDocuments(string_view raw_query, DocumentStatus status) const {
    return FindTopDocuments(raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
        return document_status == status;
        });
}

vector<Document> SearchServer::FindTopDocuments(string_view raw_query) const {
    return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

int SearchServer::GetDocumentCount() const {
    return documents_.size();
}

tuple<vector<string_view>, DocumentStatus> SearchServer::MatchDocument(string_view raw_query, int document_id) const {
    const auto query = ParseQuery(raw_query);

    vector<string_view> matched_words;
    for (const string& word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.push_back(*all_words_.find(word));
        }
    }
    for (const string& word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.clear();
            break;
        }
    }
    return { matched_words, documents_.at(document_id).status};
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(const std::execution::sequenced_policy&, string_view raw_query, int document_id)const {
    return MatchDocument(raw_query, document_id);
}
std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(const std::execution::parallel_policy&, string_view raw_query, int document_id)const {
    const auto query = ParseQuery(raw_query);

    vector<string_view> matched_words;
    for_each(execution::par, query.plus_words.begin(), query.plus_words.end(),
        [=, &matched_words](const string& word) {if (word_to_document_freqs_.at(word).count(document_id) && word_to_document_freqs_.count(word) != 0 ) {
        matched_words.push_back(*all_words_.find(word));
    }});

    for_each(execution::par, query.minus_words.begin(), query.minus_words.end(),
        [=, &matched_words](const string& word) {if (word_to_document_freqs_.at(word).count(document_id) && word_to_document_freqs_.count(word) != 0) {
        matched_words.clear();
    }});
    return { matched_words, documents_.at(document_id).status };
}

//PRIVATE METODS
bool SearchServer::IsStopWord(string_view word) const {
    return stop_words_.count(static_cast<string>(word)) > 0;
}

vector<string> SearchServer::SplitIntoWordsNoStop(string_view text) const {
    vector<string> words;
    for (string_view word : SplitIntoWords(text)) {
        if (!IsValidWord(word)) {
            throw invalid_argument("Word "s + static_cast<std::string>(word) + " is invalid"s);
        }
        if (!IsStopWord(word)) {
            words.push_back(static_cast<std::string>(word));
        }
    }
    return words;
}

int SearchServer::ComputeAverageRating(const vector<int>& ratings) {
    if (ratings.empty()) {
        return 0;
    }
    int rating_sum = 0;
    for (const int rating : ratings) {
        rating_sum += rating;
    }
    return rating_sum / static_cast<int>(ratings.size());
}

SearchServer::QueryWord SearchServer::ParseQueryWord(string_view text) const {
    if (text.empty()) {
        throw invalid_argument("Query word is empty"s);
    }
    string_view word = text;
    bool is_minus = false;
    if (word[0] == '-') {
        is_minus = true;
        word = word.substr(1);
    }
    if (word.empty() || word[0] == '-' || !IsValidWord(static_cast<std::string>(word))) {
        throw invalid_argument("Query word "s + static_cast<std::string>(text) + " is invalid"s);
    }
    return { static_cast<std::string>(word), is_minus, IsStopWord(static_cast<std::string>(word)) };
}

SearchServer::Query SearchServer::ParseQuery(string_view text) const {
    Query result;
    for (const string_view& word : SplitIntoWords(text)) {
        const auto query_word = ParseQueryWord(word);
        if (!query_word.is_stop) {
            if (query_word.is_minus) {
                result.minus_words.insert(query_word.data);
            }
            else {
                result.plus_words.insert(query_word.data);
            }
        }
    }
    return result;
}

double SearchServer::ComputeWordInverseDocumentFreq(const string& word) const {
    return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}

const map<string_view, double>& SearchServer::GetWordFrequencies(int document_id) const {
    return  words_frequency_in_document_.at(document_id);
}

void SearchServer::RemoveDocument(int document_id) {

    if (document_ids_.count(document_id) == 0) {
        return;
    }
    
    for (auto& [word, freq] : words_frequency_in_document_.at(document_id)) {       
        if (word_to_document_freqs_.at(word).count(document_id) != 0) {
            word_to_document_freqs_.at(word).erase(document_id);
        }
        if (word_to_document_freqs_.size() == 0) {
            word_to_document_freqs_.erase(word);
        }
    }

    documents_.erase(document_id);
    document_ids_.erase(document_id);
    words_frequency_in_document_.erase(document_id);
}


void SearchServer::RemoveDocument(const execution::sequenced_policy&, int document_id) {
    RemoveDocument(document_id);
}

void SearchServer::RemoveDocument(const std::execution::parallel_policy&, int document_id) {
    
    if (document_ids_.count(document_id) == 0) {
        return;
    }

        for_each(execution::par, words_frequency_in_document_.at(document_id).begin(), words_frequency_in_document_.at(document_id).end(),
            [= ,&document_id](const auto& a) {if (word_to_document_freqs_.at(a.first).count(document_id) != 0) {
            word_to_document_freqs_.at(a.first).erase(document_id);
            if (word_to_document_freqs_.size() == 0) {
                word_to_document_freqs_.erase(a.first);
            }
        }});

        documents_.erase(document_id);
        document_ids_.erase(document_id);
        words_frequency_in_document_.erase(document_id);

}






