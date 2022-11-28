#pragma once
#include "search_server.h"
#include "document.h"
#include <iostream>
#include <string>
#include <vector>

std::string ReadLine();

int ReadLineWithNumber();

void MatchDocuments(const SearchServer &search_server,
		const std::string &query);

void FindTopDocuments(const SearchServer &search_server,
		const std::string &raw_query);

void AddDocument(SearchServer &search_server, int document_id,
		const std::string &document, DocumentStatus status,
		const std::vector<int> &ratings);
