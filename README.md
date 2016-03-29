# Parallel-Histogram
Histogram shows the number of occurrences of different unique values. 
Different steps carried out in this algorithm were as follows:
1.The input from the file contained a lot of redundant information. Input was first read into an array Keys which had many occurrences of the same key. This Keys array was preprocessed to create an array having only unique keys. Corresponding to this, another counts array was created to store the count of each such unique keys in counts array. Total number of unique keys in the input range was 1575.
2.For each thread, a local copy of the counts array was created so that each of them could work on it in parallel without collision. 
3.The input is divided into chunks between the threads. Hence one thread worked on one such chunk at a time. And each thread updated the count of keys in its own local copy of the counts array. This implementation avoided the use of locks and barriers which would have slowed down the total execution time to a considerable amount. This step was done in parallel using 64 threads.
4.Finally the results from all the local copy of counts array was accumulated to create a final counts array. This step was also done in parallel.
