# BUILD

To compile the single-thread implementation (for low mem scenario):

	$ cd quiz
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


# Time VS Space Discussion

1. The usage of a word tree has reduced RAM consumption and boosted performance significantly;

2. If RAM is limited, use the single thread implementation to read the content of the input file in an accumulated manner. The chunk size is also configurable;

3. If RAM is sufficient, the performance could be further promoted by reducing disk I/O. Also, *in theory*, multiple threads could be forked to analyse different parts of the input file in parrallel.

However, *in practice*, multi-threads could NOT always guarantee a better performance due to contentions on pthread mutex for a given node (since the tsync_t implementation is not optimised for this application in the first place).


#Test Results

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
	
	$ time build/analysis_s test/28M.txt > log_s
	real	0m0.778s
	user	0m0.735s
	sys		0m0.040s
	
	$ diff -uP log_m_1 log_m_2
	$ diff -uP log_m_1 log_m_3
	$ diff -uP log_m_1 log_m_4
	$ diff -uP log_m_1 log_m_5
	$
	$ diff -uP log_s log_m_1
	$ 
