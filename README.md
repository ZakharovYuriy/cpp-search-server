# Search server
Educational project С++17

# Program Description
This is a search engine for lost animals. To do this, you need a search query and the documents that are being searched.
Let's say someone found a cat on the street and posted an ad about it. So a document appeared in the system "A white fluffy cat with a blue collar was found in the Perovo district in Moscow phone for communication 89222334455". The owner of the cat sent a request "White cat blue collar" and was able to return the pet.In general, the search engine can work with any queries.

# Build a Project using Cmake
To build this project on linux you need:<br>
1)If you don't have Cmake installed, install Cmake<br>
2)If the "Debug" or "Release" folders are not created:<br>

```
mkdir Debug
mkdir Release
```
3)Run the command for Debug and Release conf:<br>

```
cmake -E chdir Debug/ cmake -G "Unix Makefiles" ../ -DCMAKE_BUILD_TYPE:STRING=Debug
cmake -E chdir Release/ cmake -G "Unix Makefiles" ../ -DCMAKE_BUILD_TYPE:STRING=Release
```
4)Build command:<br>

```
cmake --build Debug/.
cmake --build Release/.
```

5)To **Run** program- go to the debug (cd Debug/) or release (cd Release/) folder and run:<br>

```
./search_server
```

**ALL in one command(Release)**:<br>

```
mkdir Release; cmake -E chdir Release/ cmake -G "Unix Makefiles" ../ -DCMAKE_BUILD_TYPE:STRING=Release && cmake --build Release/.
```

# Usage

### Before you start:
  0. Installing and configuring all required components in the development environment to run the application
  1. The use case is shown in main.cpp and test_example_functions.h 

## Description of features:

### The core of the search engine is the class:  SearchServer
 Methods of the SearchServer class:
 
- The constructor accepts a string of stop-words, for example: `"in at and"s` <br>
  The stop-word in the query is not taken into account when searching.
  
-  Adding documents to the search engine.
  `void AddDocument(int document_id, string_view document,DocumentStatus status, const vector<int> &ratings);` 
   
  document - string as: `"funny pet and nasty -rat"s`<br>
  where *"funny pet nasty"* - the words that will be searched for<br>
  *"and"* - the stop word specified in the SearchServer constructor<br>
  *"-rat"* - mius-word<br>
  Mnius-words exclude documents containing such words from the search results.<br>
  Possible DocumentStatus: `ACTUAL, IRRELEVANT, BANNED, REMOVED` <br>
  ratings -  Each document at the input has a set of user ratings. <br>
  The first digit is the number of ratings<br>
  For example:*{4 5 -12 2 1}*;<br>
  
- Document search in the search server and ranking by TF-IDF<br>
  There are 6 ways to call the function 3 multithreaded (ExecutionPolicy) and 3 single-threaded  <br>
  `FindTopDocuments (ExecutionPolicy,query)`  
  `FindTopDocuments (ExecutionPolicy,query,DocumentStatus)`  
  `FindTopDocuments (ExecutionPolicy,query,DocumentPredicate)`  
  `FindTopDocuments (query)`  
  `FindTopDocuments (query,DocumentStatus)`  
  `FindTopDocuments (query,DocumentPredicate)`<br>
  <br>
  Returns vector<Document> matching by *query*<br>
  The usefulness of words is evaluated by the concept of inverse document frequency or IDF. <br>
	This value is a property of a word, not a document. <br>
	The more documents have a word in them, the lower the IDF.<br>
	Above, place documents where the search word occurs more than once. <br>
	Here you need to calculate the term frequency or TF.<br>
	For a specific word and a specific document, this is the share that this word occupies among all.<br>
  
 - `GetDocumentCount()` - returns the number of documents in the search server<br>
  
 - `begin и end` - They will return iterators. The iterator will give access to the id of all documents stored in the search server.<br>
  
 - `tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(raw_query, document_id)`<br>
  Returns:The first object is a vector of query words that were found in the document_id document, <br>
	and the second object is the status of the document<br>

 - Method of obtaining word frequencies by document id:<br>
 `map<string, double> GetWordFrequencies(document_id)`<br>

 - Deleting a document from the search server <br>
  `RemoveDocument(document_id)`  
  `RemoveDocument(ExecutionPolicy,document_id)`  
  `RemoveDocument(ExecutionPolicy, document_id)`
  
### Additional functionality:
- Output to the standard input/output stream information about the document<br>
  `PrintDocument(document)`<br>
  `PrintMatchDocumentResult(document_id,vector<std::string_view> &words, DocumentStatus)`<br>

- Класс `LogDuration` - allows profiling<br>

- `Paginate()` - Allows you to split the results into pages<br>

- `RemoveDuplicates(SearchServer)` - allows you to remove duplicate documents<br>

- As well as other functions that provide input data processing.<br>

# System requirements:
  1. C++17(STL)
  2. GCC (MinG w64) 11.2.0  
  
# Plans for completion:
1. Add UI
2. Add the ability to read documents from files
3. Add support for JSON requests/responses

# Technology stack:
  1. The project shows knowledge of the basic principles of C++ programming:<br>
     a. **Numbers**, **strings**, **symbols**, data input/output in **console**, **conditions**, **loops**.<br>
     b. The use of basic algorithms **<algorithm>**.<br>
		 c. Using **structures**, **classes**, **lambda functions**, creating **tuples**<br>
		 d. Parsign of lines with output to the screen<br>
				String parsing with saving in the container **vector**<br>
				Allocation of string parsing in a function for further reuse <br>
				(Working with **functions** , **Arguments**, **Return result**<br>
				Query Parsing - **Links**, **Constancy**, **Deep Copying** <br>
        Stop word processing - container **set**<br>
				Adding documents to the search engine, Document Search,<br>
				Calculating the relevance of found documents - containers: **pair**, **map** <br>
		 e. Use of **Templates** and **Specialization of templates**<br>
		 f. Creating and using macros<br>
		 g. **Overloading** operators<br>
		 h. Handling **exceptions**<br>
		 i. Application of the **optional** class<br>
		 g. **Iterators**<br>
		 k. Recursion<br>
		 l. **Stack**, **Dec**<br>
     m. Working with standard input/output streams<br>
		 Working with file streams<br>
		 n. **Static**, **Automatic** and **Dynamic** placement of objects in memory<br>
		 o. Parallel work using the bible library **<execution>**<br>
		 p. Scan algorithms<br>
		 q. Asynchronous calculations using the library **<future>**<br>
		 r. Race state protection: **mutex**, **lock_guard**, **atomic**-types<br>

  2. Calculation of term frequency and inverse document frequency
	3. **Unit testing**
	4. Decomposition and debugging
	5. Creating multi-file projects
	6. Profiling
	7. Evaluation of the complexity of the program
	8. MapReduce is a concept in which the algorithm is divided into two stages:<br>
		 independent filtering and transformation of individual elements (map or transform);<br>
		 grouping of the results of the previous stage (reduce).<br>

# Description of the tests performed:  
## ManyRequests  
When there are too many requests, it takes time to process them. Requests waiting to be processed can simply "sit" in the queue and wait for the service handler to get to them.<br>
To store only the necessary queries. For example, you want to know what queries users have sent to the search server. But only relevant requests for the last day are important. That is, you are interested in the time of sending. It is not required to store requests older than a day.
One request from the user comes every minute. That is, the maximum number of requests to be stored is the number of minutes per day (1440). The appearance of 1441 requests will mean that the day has passed, the first request of the previous day is no longer interesting to us and can be deleted. To implement such a mechanism, it is convenient to use deque. A new query will be easily inserted at the end, and an outdated query will be deleted from the beginning.<br>

## Split the results into pages  
Iterators can be used far from the most obvious cases. Imagine that a search server contains hundreds of thousands or even millions of documents. Thousands of them fit the user's request. In this case, displaying all the requests on the screen at once would not be a good idea. You need to break the results into pages.<br>
Iterators will come in handy. Suppose we have a container. Then one page is a certain range of a certain size from this container. The first element is included in the page, and the last one is not. But the last one will be the first element on the next page. That is, we can get a container with the results, and then create a vector of ranges based on it, where the range will be just a pair of iterators. The first iterator will point to the beginning of the page, and the second one will point to its end.<br>
The Paginator class can be responsible for the separation by pages.<br>

## Specialization of templates  
The FindTopDocuments method can accept a more universal document filter, a predicate function, instead of the status. It, in turn, accepts the document id, status and rating and returns bool. Filtering of documents should be done before cutting off the top of five pieces.
Calling Find Top Documents without the second argument should search only for current documents.

## Duplicate search and removal function:  
Search engines face a problem — mirrors. These are copies of the site. Their number on the Internet can reach tens or hundreds. To ensure that the first pages of the search results do not consist of copies of the same site, you need to develop a deduplicator. It deletes copies from the search server. Duplicates are documents that have the same sets of words that occur. Frequency matching is optional. Word order is unimportant, and stop words are ignored. The function uses only the search server methods available at that moment.<br>
If duplicate documents are detected, the function should delete a document with a large id from the search server, and at the same time report the id of the deleted document in accordance with the output format.<br>

## Multithreading tests  
The ProcessQueries function parallelizes the processing of multiple queries to the search engine.<br>
The ProcessQueriesJoined function, like the ProcessQueries function, parallelizes the processing of several queries to the search engine, but returns a set of documents in a flat form.<br>
Often it is really enough to parallelize the processing of several requests — and the thing is in the hat. But there is another situation: one request is processed for too long, and its implementation needs to be parallelized.<br>
A multithreaded version of the RemoveDocument method in addition to the single-threaded one.<br>
A multithreaded version of the MatchDocument method in addition to the single-threaded one.<br>
A multithreaded version of the FindTopDocuments method in addition to the single-threaded one.<br>

# Features
### Throw an exception
The constructors of the SearchServer class throw an invalid_argument exception if any of the passed stop words contains invalid characters, that is, characters with codes from 0 to 31.
The addDocument method throws an invalid_argument exception in the following situations:<br>
	An attempt to add a document with a negative id;<br>
	Attempt to add a document with the id of a previously added document;<br>
	The presence of invalid characters (with codes from 0 to 31) in the text of the document being added.<br>
FindDocument methods throws an invalid_argument exception in the following situations:<br>
	There are invalid characters with codes from 0 to 31 in the words of the search query;<br>
	The presence of more than one minus sign before words that should not be in the required documents, for example,   <br>
	fluffy --cat. In the middle of the words, cons are resolved, for example: ivan-tea.<br>
The absence of text after the "minus" symbol: in the search query: "fluffy -".<br>
The MatchDocument method throws an invalid_argument exception in the same situations as the FindDocument method.<br>
The GetDocumentId method throws an out_of_range exception if the index of the transmitted document is outside the allowed range (0; number of documents).

### Iterators BEGIN END
The begin and end methods are defined. They will return iterators. The iterator will give access to the id of all documents stored in the search server.


