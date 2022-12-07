import subprocess
import numpy as np
import os 

fname = "HG01_k5_mu0.50_(20,4,5)_B0.65_N1000_00"
print('===== GREEDY MERGING ALGORITHM =====\n')
mcm_args = ('../gm/mcm_greedy.exe', fname)
mcm = subprocess.Popen(mcm_args)
mcm.communicate()

mcm_file = f'../comms/{fname}_mcm_results.dat'
mcm_result = np.loadtxt(mcm_file, delimiter=';', dtype=str)
mcm_logE = mcm_result[1]
print('GREEDY LOG E (LOAD) :', mcm_logE, '\n')
print('REMOVING:', mcm_file)
os.remove(mcm_file)

mcm_comm = f'../comms/{fname}_mcm_comms.dat'
mcm_comms = np.loadtxt(mcm_comm, dtype=str)
print('REMOVING:', mcm_comm)
os.remove(mcm_comm)


comm_list = []
for c in mcm_comms:
	x = int(c,2)
	comm_list.append(x)

print('GREEDY PARTITION: ', comm_list,'\n')
np.savetxt(f'../comms/{fname}_mcm_communities.dat', comm_list, fmt='%i')
	
print('===== SIMULATED ANNEALING ALGORITHM =====\n')
sa_args = ('../sa/saa.exe', fname)
sa = subprocess.Popen(sa_args)
sa.communicate()
