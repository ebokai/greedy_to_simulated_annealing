#include "header.h"
#include <time.h>
#include <ctime> 
#include <ratio>
#include <chrono>

int main(int argc, char **argv) {

	random_device randDevice;
    srand(randDevice());
    string fname = argv[1];

    // LOAD DATA AND INITIALIZE PARTITION
    Partition p_struct;
    p_struct = get_data(fname, p_struct);
    //p_struct = random_partition();
    p_struct = load_partition(p_struct, fname);
    p_struct.best_log_evidence = p_struct.current_log_evidence;
	p_struct.best_partition = p_struct.current_partition;

	// INITIALIZE PARAMETERS
    int f; // candidate function
    bool first_reset = true;
    bool no_improvement = true;
    double T0 = 100;
    p_struct.T = T0;
    unsigned int update_schedule = 100;
    unsigned int iterations = 0;
    unsigned int steps_since_improve = 0;
	unsigned int max_no_improve = 10000;

    // MAIN LOOP
    auto start = chrono::system_clock::now();
    for (int i = 0; i < 50000; i++){

    	iterations++;

    	// CANDIDATE SELECTION
    	// Some candidate partitions are not valid updates in certain situations. 
    	// If there are only independent communities (nc = n), these can not be split further
    	// and switching nodes leaves the partition unchanged. 
    	// If there is just one big community (nc = 1), it can not be merged with anything.
    	if (p_struct.nc == n){
    		f = 0; 
    	} else if (p_struct.nc == 1){
    		f = 1; 
    	} else {
    		f = rand()/(RAND_MAX/3);
    	}
    	
		switch(f){
		case 0: 
			p_struct = merge_partition(p_struct);
			break;
		case 1:
			p_struct = split_partition(p_struct);
			break;
		case 2:
			p_struct = switch_partition(p_struct);
			break;
		}

		// UPDATE ANNEALING TEMPERATURE 
		if (i % update_schedule == 0){
			p_struct.T = T0 / (1 + log(1 + i));
		}

		// UPDATE BEST PARTITION
		// DoubleSame checks if log-evidences are the same within tolerance EPSILON (declared in header.h)
		// If disabled, the algorithm sometimes finds neglible increases in log-evidence that are 
		// artifacts of numerical precision for communities that are the same up to re-ordering.
		if ((p_struct.current_log_evidence > p_struct.best_log_evidence) && !(DoubleSame(p_struct.current_log_evidence, p_struct.best_log_evidence))){
			p_struct.best_log_evidence = p_struct.current_log_evidence;
			p_struct.best_partition = p_struct.current_partition;
			cout << "Best log-evidence: " << p_struct.current_log_evidence << "\t@T = " << p_struct.T << endl;
			no_improvement = false;
			steps_since_improve = 0;
		} else {
			steps_since_improve++;
		}

		// RESET PARTITION TO RANDOM PARTITION 
		// For partitions loaded from file, sometimes the algorithm is not able to find a path 
		// to a more optimal partition. In that case, the algorithm is restarted from a random partition. 
		// In many cases, this allows the algorithm to find a more optimal solution. This mostly happens
		// if the loaded partition consists of all independent communities. 
		if (steps_since_improve > max_no_improve){
			cout << "Maximum number of steps without improvement in log-evidence." << endl;
			if ((first_reset) && (no_improvement)){
				p_struct = random_partition(p_struct);
				steps_since_improve = 0;
				first_reset = false;
			} else {
				break;
			}
		}

    }

    auto end = chrono::system_clock::now();
	chrono::duration<double> elapsed = end - start;
	cout << "Iterations per second: " << static_cast <double> (iterations) / elapsed.count() << endl;

	for(int i = 0; i < n; i++){
		uint64_t community = p_struct.best_partition[i];
		if (bitset<n>(community).count() > 0){
			cout << bitset<n>(community) << endl;
		}
	}
    
    

    return 0;


}