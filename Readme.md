# Hardware Trojan Horse

### Contact

1. Maintainer: Tien-Hung, Tseng (Eric, 曾天鴻)
2. Email: eric830303@gmail.com

### How to compile the program on your terminal
		TienHungTseng@IP-194-111:~/Desktop/HT (code)    $ make
		-----------------------------------
 		Project Name: HTH               
 		Check: MINISATS(dir) --> Exist !
 		Check: benchmark(dir) --> Exist !
 		Check: quality(dir) --> Exist !
 		Your OS: Mac OS X (Darwin)      
		-----------------------------------
		Compiled src/Data.cpp successfully!
		...
### How to execute the program on your terminal
		TienHungTseng@IP-194-111:~/Desktop/HT (code)    $ ./research s38417
The program will follow the setting in "/parameter/Parameter.txt":

		Attack_yr   5
		thershold   0.36
		edge error  0.03
		PLUS fixed  7
		TIGHT   1.000001
		FINAL   0
		MONTE YES
		Convergent_Year 20
		Error_yr    0.5
		MDS_Times   20
		Instance_Ctr    5
		Vth_SD(PV)  0.02
		A_C 0.0039
		Exp 0.2
		Tc_Margin   1.0000
		Alpha   0.5

### Parameters for each benchmark
| Benchmark Name	| Lifetime | R^2 Threshold | R^2 Error |
|:----------------:|:------------------:|:------------------:|:------------------:|
|s38417	| 2	 | 0.65 | 0.01 |
|		| 3 | 0.50 | 0.02 |
|		| 4 | 0.40 | 0.02 |
|		| 5 | 0.36 | 0.03 |
|		| 6 | 0.36 | 0.03 |
		
| Benchmark Name    | Lifetime | R^2 Threshold | R^2 Error |
|:----------------:|:------------------:|:------------------:|:------------------:|
|vga_lcd| 2 | 0.36 | 0.03 |
|		| 3 | 0.36 | 0.03 |
|		| 4 | 0.36 | 0.03 |
|		| 5 | 0.36 | 0.03 |
|		| 6 | 0.36 | 0.03 |
		
| Benchmark Name    | Lifetime | R^2 Threshold | R^2 Error |
|:----------------:|:------------------:|:------------------:|:------------------:|
| leo3mp	| 2	 | N/A | N/A |
|		| 3 | 0.80 | 0.01 |
|		| 4 | 0.80 | 0.01 |
|		| 5 | 0.80 | 0.01 |
|		| 6 | 0.80 | 0.01 |

| Benchmark Name    | Lifetime | R^2 Threshold | R^2 Error |
|:----------------:|:------------------:|:------------------:|:------------------:|
|des_perf| 2| 0.36 | 0.03 |
|		| 3 | 0.36 | 0.03 |
|		| 4 | 0.36 | 0.03 |
|		| 5 | 0.36 | 0.03 |
|		| 6 | 0.36 | 0.03 |

Benchmark Name	| Lifetime | R^2 Threshold | R^2 Error
|:----------------:|:------------------:|:------------------:|:------------------:|
netcard| 2 | N/A | N/A |
|		| 3 | N/A | N/A |
|		| 4 | N/A | N/A |
|		| 5 | 0.80 | 0.03 |
|		| 6 | 0.80 | 0.03 |
			
