#include "header.h"

double icc_evidence(uint64_t community, map<uint64_t, int> &data, int &N){

	double logE = 0;
	int k;

	map<uint64_t, int> pdata = build_pdata(data, community);
	map<uint64_t, int>::iterator it;

	unsigned int rank = (bitset<n>(community).count());
	double rank_pow = (1 << (rank - 1));
	double pf = lgamma(rank_pow) - lgamma(N + rank_pow);

	logE += pf;
	for (it = pdata.begin(); it != pdata.end(); it ++){
			k = it -> second;
			logE += lgamma(k + 0.5) - lgamma(0.5);
		}

	//cout << "ICC rank: " << rank << endl;

	return logE;
}

map<uint64_t, int> build_pdata(map<uint64_t, int> &data, uint64_t pbit){

	map<uint64_t, int> pdata;
	map<uint64_t, int>::iterator it;

	int ks; 
	uint64_t state, mask_state;

	for(it = data.begin(); it != data.end(); it++){

		state = it -> first;
		ks = it -> second;

		mask_state = state & pbit;
		pdata[mask_state] += ks;

	}

	return pdata;

}

void print_partition(Partition p_struct){
	uint64_t community;
	for (unsigned int i = 0; i < p_struct.current_partition.size(); i++){
		community = p_struct.current_partition[i].first;
		cout << bitset<n>(community) << endl;
	}
}

double get_voi(vector<pair<uint64_t, double>> P1, vector<pair<uint64_t, double>> P2){

	int n1 = P1.size();
	int n2 = P2.size();

	uint64_t c1, c2;
	double mut = 0;
	double ent = 0;
	double voi, p1, p2, p12;

	for (int i = 0; i < n1; i++){

		c1 = P1[i].first;
		p1 = static_cast <double> (bitset<n>(c1).count()) / static_cast <double> (n);

		for (int j = 0; j < n2; j++){
			
			c2 = P2[j].first;
			p2 = static_cast <double> (bitset<n>(c2).count()) / static_cast <double> (n);
			p12 = static_cast <double> (bitset<n>(c1 & c2).count()) / static_cast <double> (n);

			if (p12 > 0){
				ent += p12 * log(p12);
				mut += p12 * log(p12/p1/p2);
			}
		}
	}

	voi = 1 + mut/ent;

	return voi;

}
