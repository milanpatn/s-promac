# s-promac
Secure Proactive MPC based MAC Protocol for Cognitive Radio Networks

Description of various folders.
------------------------------

CR_Src - Root folder

  S-PROMAC_PUEA    - Distributed S-ProMAC with PUEA Attack
  S-PROMAC_SSDF_1  - Master Slave S-ProMAC with SSDF Attack causing SU backoff
  S-PROMAC_SSDF_2  - Master Slave S-ProMAC with SSDF Attack causing PU interference
  FPFC_PUEA    	   - Distributed ProMAC with PUEA Attack
  FPFC_SSDF_1      - Master Slave ProMAC with SSDF Attack causing SU backoff
  FPFC_SSDF_2      - Master Slave ProMAC with SSDF Attack causing PU interference
  RCA     	   - Reactive Channel Allocation (RCA)
  PO-MAC    	   - PO-MAC (By default PO-MAC is distributive) 
 
Execution.
---------- 

./rem 
g++ -g *.cpp *.h -lpthread
./a.out 

Example:

1. Remove the temporary files

milan@milan-Inspiron-N5110:~/Dropbox/CR_src/D-FPFC$ ./rem
rm: cannot remove ‘*.gch’: No such file or directory
rm: cannot remove ‘*.out’: No such file or directory
rm: cannot remove ‘10*’: No such file or directory
rm: cannot remove ‘*hty*’: No such file or directory

2.Compilation

milan@milan-Inspiron-N5110:~/Dropbox/CR_src/FPFC_PUEA$ g++ -g *.cpp *.h -lpthread
node.cpp: In function ‘void x()’:
node.cpp:31:14: warning: deprecated conversion from string constant to ‘char*’ [-Wwrite-strings]
   

3. Execution

milan@milan-Inspiron-N5110:~/Dropbox/CR_src/RCA_PUEA$ ./a.out 
				startTime = 1474994655
Scan Time  0.000216
Num of Channels = 20
Num of PU = 20
Num of SU = 20
simtime = 91
Busy time  35.659260  	Free time  0.000000  
Busy time  68.350420  	Free time  0.000000  
Busy time  70.487360  	Free time  0.000000  
Busy time  53.741760  	Free time  0.000000  
Busy time  62.365660  	Free time  0.000000  
Busy time  58.753320  	Free time  0.000000  
Busy time  61.569080  	Free time  0.000000  
Busy time  61.115360  	Free time  0.000000  
Busy time  61.593320  	Free time  0.000000  
Busy time  49.793460  	Free time  0.000000  
Busy time  62.826280  	Free time  0.000000  
Busy time  54.960880  	Free time  0.000000  
Busy time  45.852100  	Free time  0.000000  
Busy time  58.649400  	Free time  0.000000  
Busy time  39.503660  	Free time  0.000000  
Busy time  41.938440  	Free time  0.000000  
Busy time  38.222200  	Free time  0.000000  
Busy time  51.601340  	Free time  0.000000  
Busy time  39.455160  	Free time  0.000000  
Busy time  38.069800  	Free time  0.000000  
Avg busyTime of Primary  = 33.022913
Avg busyTime of secondary  = 19.702500
Avg freetime of secondary  = 21.823508
Channel Utilisation  57.940152 
Avg Number of Backoff's = 6
Runtime of Secondary = 58
Number of Failures = 5016
779.949784	 0.304530Tue Sep 27 22:15:46 2017

