# Build

To compile the single-thread implementation (for low mem scenario):

	$ cd quiz
	$ git checkout master
	$ cmake .
	$ VERBOSE=1 make

To compile the multi-threads implementation (in the hope to boost performance):

	$ cmake -DCMAKE_BUILD_TYPE=THREADS .
	$ VERBOSE=1 make

To clean up:

	$ make quiz-clean
	
# Run Test Cases

	$ build/analysis_s <input file> [chunk size] > output.txt

	Or

	$ build/analysis_m <input_file> [num of threads] > output.txt


	If the chunk size or the number of threads is omitted, their default value is 4096 and 4 respectively.

	Unzip the test folder to get some example input files:

	$ tar xvf test.tar.gz
	$ ll test -h
		total 28M
		-rw-r--r--. 1 cao cao  28M May 14 16:20 28M.txt
		-rw-rw-r--. 1 cao cao 353K May 14 19:17 353K.txt
		-rw-rw-r--. 1 cao cao   49 May 14 21:46 small.txt


To check memory usage of this program:

	$ valgrind 	--leak-check=full --log-file=analysis.val <program> <option>


# Discussion

1. The usage of a word tree has reduced RAM consumption and boosted performance significantly;

2. If RAM is limited, use the single thread implementation to read the content of the input file in an accumulated manner. The chunk size is also configurable via corresponding parameter on the command line;

3. Whereas when RAM is sufficient, the performance could be further promoted by reducing disk I/O. Also, in theory, multiple threads could be forked to analyse different parts of the input file in parrallel.

4. However, synchronisation among threads don't come without a cost. Experiments reveal that having *one and only one* mutex for the entire subtree as rooted by a particular alphabet can yield a much better performance than equipping each node with its own mutex, which might be desirable when scalability became a priority.

#Test Results

## Single-thread implementation

	$ time build/analysis_s test/28M.txt > log_s
	real	0m0.778s
	user	0m0.735s
	sys		0m0.040s

## Multi-thread implementation (on master branch)

	$ time build/analysis_m test/28M.txt 1 > log_m_1

	real	0m0.918s
	user	0m0.852s
	sys		0m0.053s
	
	$ time build/analysis_m test/28M.txt 2 > log_m_2
	
	real	0m0.736s
	user	0m1.042s
	sys		0m0.138s
	
	$ time build/analysis_m test/28M.txt 3 > log_m_3
	
	real	0m0.659s
	user	0m1.181s
	sys		0m0.168s
	
	$ time build/analysis_m test/28M.txt 4 > log_m_4
	
	real	0m0.624s
	user	0m1.298s
	sys		0m0.239s
	
	$ time build/analysis_m test/28M.txt 5 > log_m_5
	
	real	0m0.626s
	user	0m1.280s
	sys		0m0.257s
	
	$ time build/analysis_m test/28M.txt 6 > log_m_6
	
	real	0m0.986s
	user	0m2.108s
	sys		0m0.615s
	
	$ time build/analysis_m test/28M.txt 7 > log_m_7
	
	real	0m0.953s
	user	0m2.069s
	sys		0m0.555s
	
	$ time build/analysis_m test/28M.txt 8 > log_m_8
	
	real	0m0.646s
	user	0m1.284s
	sys		0m0.298s
	
	$ time build/analysis_m test/28M.txt 9 > log_m_9
	
	real	0m0.629s
	user	0m1.251s
	sys		0m0.309s
	
	$ time build/analysis_m test/28M.txt 10 > log_m_10
	
	real	0m0.648s
	user	0m1.283s
	sys		0m0.313s

As we can see, multi-thread implementation can further promote performance, but it may turn worse with the increasing contention on the mutex.

## Multi-thread implementation (on devel branch)
	
	$ time build/analysis_m test/28M.txt 1 > log_m_1
	real	0m1.177s
	user	0m1.080s
	sys		0m0.093s
	
	$ time build/analysis_m test/28M.txt 2 > log_m_2
	real	0m0.864s
	user	0m1.230s
	sys		0m0.174s
	
	$ time build/analysis_m test/28M.txt 3 > log_m_3
	real	0m0.916s
	user	0m1.626s
	sys		0m0.333s
	
	$ time build/analysis_m test/28M.txt 4 > log_m_4
	real	0m0.791s
	user	0m1.661s
	sys		0m0.366s
	
	$ time build/analysis_m test/28M.txt 5 > log_m_5
	real	0m1.723s
	user	0m3.122s
	sys		0m0.930s

As we can see, performance suffers if the granularity of synchronisation is too small.
