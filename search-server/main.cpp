#include "test_example_functions.h"
#include "process_queries.h"
#include "search_server.h"
#include <iostream>
#include <string>
#include <vector>

int main() {
	//basic tests
	TestSearchServer();

	ManyRequests();
	TestPaginate();
	SpecializationOfTemplates();
	TestRemoveDuplicates();
	SimpleTestProcessQueries();
	StressTestProcessQueries();
	SimpleTestProcessQueriesJoined();
	StressTestProcessQueriesJoined();
	SimpleTestRemoveDocument();
	StressTestRemoveDocument();
	TestDifferentVersionsMatchDocument();
	TestDifferentVersionsFindTopDocuments();
	StressTestDifferentVersionsFindTopDocuments();
    return 0;
}
