import subprocess
import numpy as np
import os 

fname = "HG01_k5_mu0.50_(20,4,5)_B0.65_N1000_00"
print('===== GREEDY MERGING ALGORITHM =====\n')
mcm_args = ('../gm/mcm_greedy.out', fname)
mcm = subprocess.Popen(mcm_args)
mcm.communicate()

mcm_file = f'../comms/{fname}_mcm_results.dat'
mcm_result = np.loadtxt(mcm_file, delimiter=';', dtype=str)
mcm_logE = mcm_result[1]
print('GREEDY LOG E (LOAD) :', mcm_logE, '\n')
print('REMOVING:', mcm_file)
os.remove(mcm_file)

mcm_comm = f'../comms/{fname}_mcm_comms.dat'
mcm_comms = np.loadtxt(mcm_comm, delimiter='\t', dtype=str)
mcm_comms = mcm_comms[:,1]
print('REMOVING:', mcm_comm)
os.remove(mcm_comm)

comm_map = np.zeros(20)
comm_list = []
for c in mcm_comms:
	x = sorted([int(19-i) for i, y in enumerate(c.strip()) if y == '1'])
	comm_list.append(x)

print('GREEDY PARTITION: ', comm_list,'\n')
for i,c in enumerate(comm_list):
	comm_map[np.array(c)] = i  
np.savetxt(f'../comms/{fname}_mcm_communities.dat', comm_map, fmt='%i')
	
print('===== SIMULATED ANNEALING ALGORITHM =====\n')
sa_args = ('../sa/simulated_annealing.out', fname)
sa = subprocess.Popen(sa_args)
sa.communicate()