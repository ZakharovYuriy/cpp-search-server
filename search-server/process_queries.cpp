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
    const SearchServer& search_server,
    const std::vector<std::string>& queries) {
    vector<vector<Document>> result(queries.size());
    transform(
        execution::par,  // параллельная реализация
        queries.begin(), queries.end(), result.begin(),  // входной диапазон
        [&search_server](string c) { return search_server.FindTopDocuments(execution::par, c); }  // map-операция
    );
    return result;

}

std::list<Document> ProcessQueriesJoined(
    const SearchServer& search_server,
    const std::vector<std::string>& queries) {
   /* ВАРИАНТ1
   * 
   * */

    /* {
        LOG_DURATION("vector->list"s);
         list<Document> result1;
        for (auto list1 : ProcessQueries(search_server, queries)) {           
            for (const auto& doc : list1) {
                result1.push_back(doc);
            }
        };
    }*/

    /* ВАРИАНТ2
    * 
    * */
    /* {
        LOG_DURATION("vector->vector"s);
        vector<vector<Document>> result1(queries.size());
        transform(
            execution::par,  // параллельная реализация
            queries.begin(), queries.end(), // входной диапазон
            result1.begin(),  // начало выходного контейнера
            [&search_server](string c) { return search_server.FindTopDocuments(c); }  // map-операция
        );

        vector<Document> result;
        for (const auto& list1 : result1){
            result.insert(result.end(),list1.begin(),list1.end());
        }; 
    }   */

    /* ВАРИАНТ3
       **/
       
    /* { LOG_DURATION("list->list"s);
        list<list<Document>> result(queries.size());
       transform(
           execution::par,  // параллельная реализация
           queries.begin(), queries.end(), // входной диапазон
           result.begin(),  // начало выходного контейнера
           [&search_server](string c) { 
                auto a=search_server.FindTopDocuments(c); 
                return list<Document>(a.begin(),a.end());
           }  // map-операция
       );
         
       list<Document> result1;
       for (const auto& list1 : result) {
           for (const auto& doc : list1) {
               result1.emplace_back(doc);
           }
       };
   }*/

   /* ВАРИАНТ4 (самый быстрый вариант)
      *
      * */
   //{
  //     LOG_DURATION("transform_reduce_list"s);
       list<list<Document>> result(queries.size());
       list<Document> result1;
       return transform_reduce(
           execution::par,  // параллельная реализация
           queries.begin(), queries.end(), // входной диапазон
           result1,  // начальное значение 
           [](list<Document> a, list<Document> b) {
               for (const auto& doc : b) {
                   a.emplace_back(doc);
               }
               return a;
           },//reduce-операция
           [&search_server](string c) {
               auto a = search_server.FindTopDocuments(c);
               return list<Document>(a.begin(), a.end());
           }  // map-операция
       );

   //}
   //list<Document> result1;
    //return result1;
}
