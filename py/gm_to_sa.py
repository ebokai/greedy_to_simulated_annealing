import subprocess
import numpy as np
import os 

fname = "HG01_k5_mu0.50_(20,4,5)_B0.65_N1000_00"

mcm_args = ('../gm/mcm_greedy.out', fname)
mcm = subprocess.Popen(mcm_args)
mcm.communicate()

mcm_file = f'../comms/{fname}_mcm_results.dat'
mcm_result = np.loadtxt(mcm_file, delimiter=';', dtype=str)
mcm_logE = mcm_result[1]
print('MCM BEST LOG E:', mcm_logE)
print('REMOVING:', mcm_file)
os.remove(mcm_file)

mcm_comm = f'../comms/{fname}_mcm_comms.dat'
mcm_comms = np.loadtxt(mcm_comm, delimiter='\t', dtype=str)
mcm_comms = mcm_comms[:,1]
os.remove(mcm_comm)

comm_map = np.zeros(20)
comm_list = []
for c in mcm_comms:
	x = sorted([int(19-i) for i, y in enumerate(c.strip()) if y == '1'])
	comm_list.append(x)

print(comm_list)
for i,c in enumerate(comm_list):
	comm_map[np.array(c)] = i  
print(comm_map)
np.savetxt(f'../comms/{fname}_mcm_comm.dat', comm_map, fmt='%i')
	

sa_args = ('../sa/simulated_annealing.out', fname)
sa = subprocess.Popen(sa_args)
sa.communicate()