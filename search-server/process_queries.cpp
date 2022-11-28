#include "search_server.h"
#include "process_queries.h"
#include "document.h"
#include <string>
#include <vector>
#include <execution>
#include <list>
#include "log_duration.h"

using namespace std;

std::vector<std::vector<Document>> ProcessQueries(
		const SearchServer &search_server,
		const std::vector<std::string> &queries) {
	vector<vector<Document>> result(queries.size());
	transform(execution::par, queries.begin(), queries.end(), result.begin(),
			[&search_server](string c) {
				return search_server.FindTopDocuments(execution::par, c);
			}
	);
	return result;

}

std::list<Document> ProcessQueriesJoined(const SearchServer &search_server,
		const std::vector<std::string> &queries) {
	/* {
	 LOG_DURATION("vector->list"s);
	 list<Document> result1;
	 for (auto list1 : ProcessQueries(search_server, queries)) {
	 for (const auto& doc : list1) {
	 result1.push_back(doc);
	 }
	 };
	 }*/

	/* �������2
	 *
	 * */
	/* {
	 LOG_DURATION("vector->vector"s);
	 vector<vector<Document>> result1(queries.size());
	 transform(
	 execution::par,  // ������������ ����������
	 queries.begin(), queries.end(), // ������� ��������
	 result1.begin(),  // ������ ��������� ����������
	 [&search_server](string c) { return search_server.FindTopDocuments(c); }  // map-��������
	 );

	 vector<Document> result;
	 for (const auto& list1 : result1){
	 result.insert(result.end(),list1.begin(),list1.end());
	 };
	 }   */

	/* �������3
	 **/

	/* { LOG_DURATION("list->list"s);
	 list<list<Document>> result(queries.size());
	 transform(
	 execution::par,  // ������������ ����������
	 queries.begin(), queries.end(), // ������� ��������
	 result.begin(),  // ������ ��������� ����������
	 [&search_server](string c) {
	 auto a=search_server.FindTopDocuments(c);
	 return list<Document>(a.begin(),a.end());
	 }  // map-��������
	 );

	 list<Document> result1;
	 for (const auto& list1 : result) {
	 for (const auto& doc : list1) {
	 result1.emplace_back(doc);
	 }
	 };
	 }*/

	/* �������4 (����� ������� �������)
	 *
	 * */
	//{
	//     LOG_DURATION("transform_reduce_list"s);
	list<list<Document>> result(queries.size());
	list<Document> result1;
	return transform_reduce(execution::par, queries.begin(), queries.end(),
			result1, [](list<Document> a, list<Document> b) {
				for (const auto &doc : b) {
					a.emplace_back(doc);
				}
				return a;
			},
			[&search_server](string c) {
				auto a = search_server.FindTopDocuments(c);
				return list<Document>(a.begin(), a.end());
			}
	);

	//}
	//list<Document> result1;
	//return result1;
}
