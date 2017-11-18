
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
//////////////////////////////////////////////////////////////////
//          Varaible Declaration                                //
//////////////////////////////////////////////////////////////////
double **EdgeA          ;
double **EdgeB          ;
double **cor            ;
double **ser            ;
double info[5]          ;
double ERROR = 1.0      ;
double PVRange = 0.00   ;
double PLUS = 0.0       ;
double tight = 1.000001 ;
double arc_thd = 0      ;
double year = 0         ;
struct info *_sInfo = NULL;
double period = 0       ;
//////////////////////////////////////////////////////////////////
vector<PATH>  PathR     ;
vector<PATH*> PathC     ;
vector<CIRCUIT> Circuit ;
//////////////////////////////////////////////////////////////////
int    Q_mode           ;
int    TotalTimes       ;
int    Threshold        ;
int    R_Times          ;
int    R_Thre           ;
int    L_Times          ;
int    L_Thre           ;
int    Ref_Times        ;
int    Ref_Thre         ;
int    Qal_Times        ;
int    Qal_Thre         ;
int    FINAL   = 0      ;
int    trylimit= 0      ;
int    reftime = 0      ;
int    PVtimes = 0      ;
bool   monte_s = false  ;
string filename= ""     ;
//////////////////////////////////////////////////////////////////

map< double, double>pvtoSv  ;
////////////////////////////////////////////////////////////////////////////////////
//              Main                                                              //
////////////////////////////////////////////////////////////////////////////////////
int main( int argc, char* argv[] )
{
    if( argc < 6 )
    {
        printf("./research [ckt] [Desired Year] [restart times][refine times][PV times][ERROR limit]  \n");
        return 0;
    }
    
    //------------ Reading ----------------------------------------------------------------
    ReadParameter( argc, argv )     ;//Read Parameter.txt
    ReadCircuit( filename )         ;//Read *.vg
    Circuit[0].PutClockSource()     ;
    ReadPath_l(filename)            ;//Read *.rpt
    ReadVth_pv_Sv()                 ;//Read Vth_pv_Sv.txt
    ReadAgingData()                 ;//AgingRate.txtRead *.cp
    AdjustConnect()                 ;
    //------------ Cand,Mine,Safe -----------------------------------------------------------
    CheckPathAttackbility( ) ;
    ReadCpInfo( filename )          ;//Read *.cp
    
    HASHTABLE *hptr = new HASHTABLE(16,(unsigned)PathC.size()) ;
    //------------- Cal Original LT ----------------------------------------------------------
    CheckOriLifeTime()  ;
    printSetting()      ;
    //------------- Variables Declaraion -----------------------------------------------------
    bool *  bestnode = new bool[PathC.size()] ;
    chrono::steady_clock::time_point starttime, endtime,  pre_time, end_time;
    chrono::duration<double> TotalTime, MDSTime, TotalMDSTime ;
    pre_time = chrono::steady_clock::now();
    _sInfo = new struct info() ;
    //--------------- Main Area ---------------------------------------------------------------
    for( int tryi = 0 ; tryi < trylimit; tryi++ )
    {
        starttime = chrono::steady_clock::now();
        printf( YELLOW "\nRound : %d/%d--------------------------\n" RESET , tryi, trylimit ) ;
        //------------- [1] MDS ----------------------------------------------------------------
        if( !ChooseVertexWithGreedyMDS( year, false, hptr )  )
        {
            printf( RED "No Dominate Set! \n" RESET )      ;
            ChooseVertexWithGreedyMDS( year, true , hptr ) ;
            continue    ;
        }
        GenerateSAT("./CNF/sat.cnf", year )             ;
        _sInfo->oridccs = CallSatAndReadReport(0)       ;
        ChooseVertexWithGreedyMDS( year, true , hptr )  ;
        if( !_sInfo->oridccs )
        {
            if( _sInfo->bestup < 10 && _sInfo->bestlow > 1 )
            {
                printf( RED "Current Best Q = %f ~ %f \n", _sInfo->bestup, _sInfo->bestlow ) ;
            }
            printf( RED "No Solution!\n" RESET ) ;
            endtime = chrono::steady_clock::now();
            MDSTime = chrono::duration_cast<chrono::duration<double>>(endtime - starttime);
            cout << "Iteration Time = " << CYAN << MDSTime.count() << RESET << endl ;
            continue   ;
        }
        else
        {
            printf( GREEN "A Solution Exist\n" RESET ) ;
        }
        
           
        //------------- [2] Add Node -------------------------------------------------------------
        //AddNode( )                      ;
        //------------- [3] Remove Additional DCCs --------------------------------------------------
        RemoveAdditionalDCC( bestnode ) ;
        //------------- [4] Reverse Current Soltion ---------------------------------------------------
        ReverseSol( )                   ;
        
        endtime = chrono::steady_clock::now();
        MDSTime = chrono::duration_cast<chrono::duration<double>>(endtime - starttime);
        cout << "Iteration Time = " << CYAN << MDSTime.count() << RESET << endl ;
        TotalMDSTime += MDSTime ;
    }//for(tryi)
        
    if( _sInfo->bestup > 10 )
    {
        printf( "NO SOLUTION! \n" );
        return 0 ;
    }
    
    //------------- PV Monte Simulation ---------------------------------------------------------
    printf( CYAN"\n\n[Info] Begin PV-Simulation.... \n" )       ;
    printSetting()                          ;
    printDCCLocation()                      ;
    CallSatAndReadReport(1)                 ;
    PV_Monte_Simulation( _sInfo->bestup, _sInfo->bestlow  ) ;
    end_time = chrono::steady_clock::now()  ;
    TotalTime = chrono::duration_cast<chrono::duration<double>>( end_time - pre_time );
    //------------- Release Memory --------------------------------------------------------------
    release( hptr ) ;
    //------------- Show Final Results ----------------------------------------------------------
    printf( CYAN"--------------Final Result------------------------------------\n") ;
    printSetting()                              ;
    printDCCLocation()                          ;
    cout << CYAN << "Total execution Time = " << GREEN << TotalTime.count() << RESET << endl ;
    //RefineResult( year, true )                  ;//設為true才不會return數值.
    
    
    
    printf("\n");
    return 0;
}

