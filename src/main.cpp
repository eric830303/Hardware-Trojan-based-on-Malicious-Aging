
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <time.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <math.h>
#include "circuit.h"
#include "aging.h"
#include "Data.h"

#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define RESET   "\x1b[0m"

using namespace std     ;

////////////////////////////////////////////////////////////////////////////////////
//              Main                                                              //
////////////////////////////////////////////////////////////////////////////////////
int main( int argc, char* argv[] )
{
    chrono::steady_clock::time_point starttime, endtime,  pre_time, end_time, attktime;
    chrono::duration<double> TotalTime, MDSTime, TotalMDSTime, attackable ;
    
    string message ;
    //------------ Reading ----------------------------------------------------------------
    pre_time = chrono::steady_clock::now();
    CIRCUIT circuit ;
    if( !circuit.ReadParameter( argc, argv, message ) ) return -1 ;
    
    circuit.ReadCircuit( )          ;
    circuit.PutClockSource( )       ;
    circuit.ReadTimingReport( )     ;
    circuit.ReadAgingData( )        ;
    circuit.AdjustConnect( )        ;
    //------------ Cand,Mine,Safe -----------------------------------------------------------
    circuit.PathClassify( )        ;
    circuit.ReadCpInfo( )          ;//Read *.cp
    attktime   = chrono::steady_clock::now( ) ;
    attackable = chrono::duration_cast<chrono::duration<double>>(attktime - pre_time);
    
    circuit.setHashTable( new HASHTABLE(16,(unsigned)(circuit.getPathCand().size()) ) ) ;
    //------------- Cal Original LT ----------------------------------------------------------
    circuit.CheckOriLifeTime()  ;
    circuit.printSetting()      ;
    //------------- Variables Declaraion -----------------------------------------------------
    
    bool nosol = true ;
    int bestdcc = 0, dccs = 0 ;
    //--------------- Main Area ---------------------------------------------------------------
    for( int tryi = 0 ; tryi < circuit.getTryLimit(); tryi++ )
    {
        starttime = chrono::steady_clock::now();

        printf( YELLOW "\n\n\n----------- Round : " RESET"%d/%d " YELLOW"-------------------\n" , tryi, circuit.getTryLimit() ) ;
        //------------- [1] MDS ----------------------------------------------------------------
        if( !circuit.MDS( false )  )
        {
            printf( "\n ==> " RED"No Dominate Set! \n" RESET )      ;
            circuit.MDS( true ) ;
            continue    ;
        }
        circuit.GenerateSAT( ) ;
        circuit.MDS( true )    ;
        circuit.CallSatAndReadReport(0);
        circuit.setOriginDCCCount( circuit.CallSatAndReadReport(0) );
        if( !circuit.getOriginDCCCount() )
        {
            dccs = circuit.getOriginDCCCount() ;
            printf( "   ==> " RED "NO Solution\n" RESET  ) ;
            endtime = chrono::steady_clock::now();
            MDSTime = chrono::duration_cast<chrono::duration<double>>(endtime - starttime);
            printf( YELLOW "---------------------------------------------\n" RESET );
            cout << "Iteration Time = " << CYAN << MDSTime.count() << RESET << endl ;
            continue   ;
        }
        else
        {
            nosol = false ;
            printf( "   ==> " GREEN "Solution Exist\n" RESET  ) ;
        }
        
        double lower = 0, upper = 0 ;
        circuit.CalQuality( upper, lower, 0/*Q_mode*/ ) ;//Calculate quality.
        
        if( circuit.BInv( circuit.getRBestUB(), circuit.getRBestLB(), circuit.getBestUB(), circuit.getBestLB(), upper, lower, circuit.getYear(),bestdcc,bestdcc,dccs) )
        {
            system("cp ./CNF/sat.cnf ./CNF/best.cnf")   ;
        }
        printf("Q = %f ~ %f (此次MDS解)\n", upper ,lower  )   ;
        printf("Q = %f ~ %f (至今最好解)\n", circuit.getBestUB() ,circuit.getBestLB() ) ;

        endtime = chrono::steady_clock::now();
        MDSTime = chrono::duration_cast<chrono::duration<double>>(endtime - starttime);
        printf( YELLOW "---------------------------------------------\n" RESET );
        cout << "Iteration Time = " << CYAN << MDSTime.count() << RESET << endl ;
        TotalMDSTime += MDSTime ;

        
    }//for(tryi)
        
    if( nosol )
    {
        printf( "NO SOLUTION! \n" );
        return 0 ;
    }
    
    //------------- PV Monte Simulation ---------------------------------------------------------
    printf( CYAN"\n\n[Info] Begin PV-Simulation.... \n" )   ;
    circuit.printSetting()                  ;
    circuit.printDCCLocation()              ;
    circuit.CallSatAndReadReport(1)         ;
    circuit.PV_Monte_Simulation()           ;
    end_time = chrono::steady_clock::now()  ;
    TotalTime = chrono::duration_cast<chrono::duration<double>>( end_time - pre_time );
    //------------- Show Final Results ----------------------------------------------------------
    printf( CYAN"--------------Final Result------------------------------------\n") ;
    circuit.printSetting()                              ;
    circuit.printDCCLocation()                          ;
    cout << CYAN << "C/M/S Finding Time   = " << GREEN << attackable.count() << RESET << endl ;
    cout << CYAN << "Total execution Time = " << GREEN << TotalTime.count()  << RESET << endl ;
    //RefineResult( year, true )                  ;//設為true才不會return數值.
    
    
    
    printf("\n");
    return 0;
}

