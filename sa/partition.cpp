#include "header.h"

Partition initialize_partition(Partition &p_struct){
	for (int i = 0; i < n; i++){
		p_struct.current_partition[i] = 0;
	}
	p_struct.current_log_evidence = 0;
	p_struct.nc = 0;
	p_struct.occupied_partitions = 0;
	p_struct.occupied_partitions_gt2_nodes = 0;

	return p_struct;

}

Partition parse_community(Partition &p_struct, uint64_t community, int i){

	unsigned int community_size = bitset<n>(community).count();
	
	p_struct.current_partition[i] = community;
	p_struct.partition_evidence[i] = icc_evidence(community, p_struct);
	p_struct.current_log_evidence += p_struct.partition_evidence[i];
	p_struct.occupied_partitions += (ONE << i);
	p_struct.nc++;
	if (community_size >= 2){
		p_struct.occupied_partitions_gt2_nodes += (ONE << i);
	}

	return p_struct;

}

Partition load_partition(Partition &p_struct, string fname){

	p_struct = initialize_partition(p_struct);

	string fpath = "../comms/" + fname + "_mcm_communities.dat";
	string line;
	ifstream comm_file(fpath);
	uint64_t community;
	int i = 0;
	while(getline(comm_file, line)){
		community = stoull(line);
		p_struct = parse_community(p_struct, community, i);
		i++;
	}

	cout << "Loaded " << p_struct.nc << " communities" << endl;
	cout << "Initial log-evidence: " << p_struct.current_log_evidence << endl;

	return p_struct;
}


Partition random_partition(Partition &p_struct){

	p_struct = initialize_partition(p_struct);

	uint64_t community; // candidate community
	uint64_t assigned = 0; // assigned nodes

	int i = 0;
	unsigned int community_size;
	
	while(bitset<n>(~assigned).count() > 0){
		community = rand(); // be careful - this adds ones beyond the n-th bit
		community = (bitset<n>(community)).to_ulong(); // force bits beyond n to be zero
		community = community - (assigned & community); // remove already assigned nodes from candidate
		assigned += community; // add newly assigned nodes
		community_size = bitset<n>(community).count();

		// avoid adding empty communities
		if (community_size > 0){

			// store community and associated log-evidence
			p_struct = parse_community(p_struct, community, i);
	
			cout << "New community: " << bitset<n>(community) << endl;
			cout << "Log-evidence: " << p_struct.partition_evidence[i] << endl;
			cout << "Assigned nodes: " << bitset<n>(assigned) << endl;
			cout << endl;
			i++;
		}
	}

	cout << "Generated " << p_struct.nc << " communities" << endl;
	cout << "Initial log-evidence: " << p_struct.current_log_evidence << endl;
 
	return p_struct;
}


Partition merge_partition(Partition &p_struct){

	if (p_struct.nc <= 1){return p_struct;} // can't merge one community

	// choose two valid random communities
	unsigned int p1 = randomBitIndex(p_struct.occupied_partitions);
	unsigned int p2 = p1;
	while (p1 == p2){
		p2 = randomBitIndex(p_struct.occupied_partitions);
	}

	// calculate log-evidence of merged community
	uint64_t merged_community = p_struct.current_partition[p1] + p_struct.current_partition[p2];
	double merged_evidence = icc_evidence(merged_community, p_struct);
	double delta_log_evidence = merged_evidence - p_struct.partition_evidence[p1] - p_struct.partition_evidence[p2];
	
	// metropolis acceptance probability
	double p = exp(delta_log_evidence/p_struct.T);
	double u = static_cast <double> (rand()) / static_cast <double> (RAND_MAX);
	
	if (p > u){
		p_struct.current_partition[p1] = merged_community;
		p_struct.current_partition[p2] = 0;
		p_struct.partition_evidence[p1] = merged_evidence;
		p_struct.partition_evidence[p2] = 0;
		p_struct.occupied_partitions -= (ONE << p2); // remove empty community 
		p_struct.current_log_evidence += delta_log_evidence; // update total log-evidence
		p_struct.nc -= 1;

		// update communities of size >= 2 (for split & switch)
		if ((p_struct.occupied_partitions_gt2_nodes & (ONE << p2)) == (ONE << p2)){
			p_struct.occupied_partitions_gt2_nodes -= (ONE << p2);
		}
		if ((p_struct.occupied_partitions_gt2_nodes & (ONE << p1)) == 0){
			p_struct.occupied_partitions_gt2_nodes += (ONE << p1);
		}	
	}

	return p_struct;
}

Partition split_partition(Partition &p_struct){

	if (p_struct.nc == n){return p_struct;} // can't split independent communities
	if (p_struct.occupied_partitions_gt2_nodes == 0){return p_struct;} // can't split communities of size 1

	// choose random valid community
	unsigned int p1 = randomBitIndex(p_struct.occupied_partitions_gt2_nodes);
	uint64_t community = p_struct.current_partition[p1];

	uint64_t mask = rand();
	uint64_t c1 = (community & mask);
	uint64_t c2 = (community & (~mask));

	// masking shouldn't assign all nodes to one community
	while ((bitset<n>(c1) == 0) || (bitset<n>(c2) == 0)){
		mask = rand();
		c1 = (community & mask);
		c2 = (community & (~mask));
	}

	double log_evidence_1 = icc_evidence(c1, p_struct);
	double log_evidence_2 = icc_evidence(c2, p_struct);
	double delta_log_evidence = log_evidence_1 + log_evidence_2 - p_struct.partition_evidence[p1];
	double p = exp(delta_log_evidence/p_struct.T);
	double u = static_cast <double> (rand()) / static_cast <double> (RAND_MAX);

	if (p > u){
		// find empty spot for second split community
		bitset<n> not_occupied(~p_struct.occupied_partitions);
		uint64_t not_occupied_int = not_occupied.to_ulong();
		unsigned int p2 = randomBitIndex(not_occupied_int);

		p_struct.current_partition[p1] = c1;
		p_struct.partition_evidence[p1] = log_evidence_1;
		p_struct.current_partition[p2] = c2;
		p_struct.partition_evidence[p2] = log_evidence_2;
		p_struct.occupied_partitions += (ONE << p2);
		p_struct.current_log_evidence += delta_log_evidence;
		p_struct.nc += 1;

		if (bitset<n>(c1).count() <= 1){
			p_struct.occupied_partitions_gt2_nodes -= (ONE << p1);
		}
		if (bitset<n>(c2).count() >= 2){
			p_struct.occupied_partitions_gt2_nodes += (ONE << p2);
		}		
	}

	return p_struct;
}

Partition switch_partition(Partition &p_struct){

	if (p_struct.nc <= 1){return p_struct;} // can't switch node to same community 
	if (p_struct.nc == n){return p_struct;} // switching independent nodes doesn't change anything
	if (p_struct.occupied_partitions_gt2_nodes == 0){return p_struct;} // don't create empty partitions

	unsigned int p1 = randomBitIndex(p_struct.occupied_partitions_gt2_nodes);
	uint64_t c1 = p_struct.current_partition[p1];
	unsigned int node = randomBitIndex(c1);

	unsigned int p2 = p1;
	while (p1 == p2){
		p2 = randomBitIndex(p_struct.occupied_partitions);
	}
	uint64_t c2 = p_struct.current_partition[p2];

	uint64_t new_c1 = c1 - (ONE << node); // remove from community 1
	uint64_t new_c2 = c2 + (ONE << node); // add to community 2

	double log_evidence_1 = icc_evidence(new_c1, p_struct);
	double log_evidence_2 = icc_evidence(new_c2, p_struct);
	double delta_log_evidence = log_evidence_1 + log_evidence_2 - p_struct.partition_evidence[p1] - p_struct.partition_evidence[p2];
	double p = exp(delta_log_evidence/p_struct.T);
	double u = static_cast <double> (rand()) / static_cast <double> (RAND_MAX);

	if (p > u){
		p_struct.current_partition[p1] = new_c1;
		p_struct.current_partition[p2] = new_c2;
		p_struct.partition_evidence[p1] = log_evidence_1;
		p_struct.partition_evidence[p2] = log_evidence_2;
		p_struct.current_log_evidence += delta_log_evidence;

		if ((bitset<n>(c1).count() >= 2) && (bitset<n>(new_c1).count() <= 1)){
			p_struct.occupied_partitions_gt2_nodes -= (ONE << p1);
		} 
		if ((bitset<n>(c2).count() <= 1) && (bitset<n>(new_c2).count() >= 2)){
			p_struct.occupied_partitions_gt2_nodes += (ONE << p2);
		} 
	}

	return p_struct;
}

