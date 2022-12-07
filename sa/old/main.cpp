#include "header.h"
#include <time.h>
#include <ctime> 
#include <ratio>
#include <chrono>


int main(int argc, char **argv){

	srand(time(NULL));

	string fname = argv[1];
	int N = 0;

	Partition p_struct;
	Partition p_initial;

	p_struct.current_partition = load_partition(fname);
	p_struct.data = get_data(N, fname);

	// STORE GROUND TRUTH AND BEST GREEDY FOR VOI
	p_initial.current_partition = fixed_partition(4,5);
	p_initial.best_partition = p_struct.current_partition;

	// CALCULATE GROUND TRUTH LOG E
	double true_logE = 0;
	true_logE += icc_evidence(31, p_struct.data, N);
	true_logE += icc_evidence(992, p_struct.data, N);
	true_logE += icc_evidence(31744, p_struct.data, N);
	true_logE += icc_evidence(1015808, p_struct.data, N);

 	// CALCULATE LOG E FOR GENERATED COMMUNITIES
	uint64_t community;
	for (unsigned int i = 0; i < p_struct.current_partition.size(); i++){
		community = p_struct.current_partition[i].first; 
		p_struct.current_partition[i] = make_pair(community, icc_evidence(community, p_struct.data, N));
		p_struct.current_logE += p_struct.current_partition[i].second;
	}

	double greedy_logE = p_struct.current_logE;
	cout << "initial log-evidence: " << p_struct.current_logE << endl;

	double T0 = 100;
	int update_schedule = 100;
	int steps_since_improve = 0;
	int max_no_improve = 10000;
	int iterations = 0;
	int n0 = 0;
	int n1 = 0;
	int n2 = 0;
	p_struct.T = T0;
	p_struct.best_partition = p_struct.current_partition;
	p_struct.best_logE = p_struct.current_logE;

	cout << "==========" << endl;

	auto start = chrono::system_clock::now();

	for (int i = 0; i < 20000; i++){

		iterations++;

		if (steps_since_improve > max_no_improve) {
			cout << "No improvement in log-evidence for " << max_no_improve << " iterations. Stopping." << endl;
			break;
		}

		int f = rand()/(RAND_MAX/3);
		// int f = 1;
		switch(f){
		case 0: 
			p_struct = merge_partition(p_struct, N);
			n0++;
			break;
		case 1:
			p_struct = split_partition(p_struct, N);
			n1++;
			break;
		case 2:
			p_struct = switch_partition(p_struct, N);
			n2++;
			break;
		}


		if (p_struct.current_logE > p_struct.best_logE){
			p_struct.best_partition = p_struct.current_partition;
			p_struct.best_logE = p_struct.current_logE;
			cout << "best log-evidence: " << p_struct.best_logE << "\t@T = " << p_struct.T << endl;
			steps_since_improve = 0;
		} else {
			steps_since_improve++;
		}

		if (i % update_schedule == 0){
			p_struct.T = T0 / (1 + log(1 + i));
		}

	}

	auto end = chrono::system_clock::now();
	chrono::duration<double> elapsed = end - start;
	cout << n0 << " merge, " << n1 << " split, " << n2 << " switch." << endl;
	cout << "Elapsed time: " << elapsed.count() << "s" << endl;
	cout << "Iterations per second: " << static_cast <double> (iterations) / elapsed.count() << endl;

	string c_path = "../comms/" + fname + "_sa_communities.dat";
	string s_path = "../stats/" + fname + "_stats.dat";
	ofstream comm_file(c_path);
	ofstream stat_file(s_path);

	for (unsigned int i = 0; i < p_struct.best_partition.size(); i++){
		community = p_struct.best_partition[i].first;
		cout << bitset<n>(community) << endl;
		comm_file << community << endl;
		}

	comm_file.close();

	
	cout << "log-evidence (true)   :" << true_logE << endl;
	stat_file << "log-evidence (true):" << true_logE << endl;
	cout << "log-evidence (greedy) :" << greedy_logE << endl;
	stat_file << "log-evidence (greedy):" << greedy_logE << endl;
	cout << "log-evidence (saa)    :" << p_struct.best_logE << endl;
	stat_file << "log-evidence (saa):" << p_struct.best_logE << endl;
	cout << "voi (true/greedy)     :" << get_voi(p_initial.current_partition, p_initial.best_partition) << endl;
	stat_file << "voi (true/greedy):" << get_voi(p_initial.current_partition, p_initial.best_partition) << endl;
	cout << "voi (true/saa)        :" << get_voi(p_initial.current_partition, p_struct.best_partition) << endl;
	stat_file << "voi (true/saa):" << get_voi(p_initial.current_partition, p_struct.best_partition) << endl;
	cout << "voi (greedy/saa)      :" << get_voi(p_initial.best_partition, p_struct.best_partition) << endl;
	stat_file << "voi (greedy/saa):" << get_voi(p_initial.best_partition, p_struct.best_partition) << endl;
	
	stat_file.close();


	return 0;
}