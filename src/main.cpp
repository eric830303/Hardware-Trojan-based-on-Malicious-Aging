
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <time.h>
#include <stdlib.h>
#include <signal.h>
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
//////////////////////////////////////////////////////////////////
vector<PATH>  PathR     ;
vector<PATH*> PathC     ;
vector<PATH*> _vPathC   ;
vector<CIRCUIT> Circuit ;
//////////////////////////////////////////////////////////////////
int    SATmode          ;
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
int    pv_times         ;
//////////////////////////////////////////////////////////////////
long int dot_ctr        ;
//////////////////////////////////////////////////////////////////
string fname            ;
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
    
    string filename = argv[1]               ;
    double year    = atof( argv[2] )           ;
    int trylimit= atoi( argv[3] )           ;
    int reftime = atoi( argv[4] )           ;
    int PVtimes = atoi( argv[5] )           ;
    if( argc == 7 ) ERROR = atof(argv[6])   ;
    else            ERROR = year*0.1        ;
    PLUS    = ERROR                         ;
    tight   = 1.000001                      ;
    fname   = argv[1]                       ;
    Q_mode  = 0                             ;
    dot_ctr = 0                             ;
    int     FINAL   = 0                     ;
    bool    monte_s = false                 ;
    vector< struct PVdata* > _vPV           ;
    pv_times = PVtimes                      ;
    //------------ Read file.vg(verilog) ----------------------------------------------------
    string vgname = "./benchmark/" + filename + ".vg"    ;
    ReadCircuit(vgname);
    //------------ Read file.rpt(Timing Report) ---------------------------------------------
    string rptname = "./benchmark/" + filename + ".rpt"  ;
    Circuit[0].PutClockSource()         ;
    ReadPath_l(rptname)                 ;
    ReadVth_pv_Sv()                     ;
    //------------ Read AgingRate.txt -------------------------------------------------------
    ReadAgingData();
    AdjustConnect();
    
    //------------ Read Parameter.txt -------------------------------------------------------
    ReadParameter( PLUS , tight , FINAL , monte_s , year ,TotalTimes ,Threshold, PVRange ) ;
    
    //------------ Define Cand,Mine,Safe ----------------------------------------------------
    CheckPathAttackbility(year, tight, true, PLUS) ;
    
    if ( PathC.size() <= 0 )
    {
        printf( "No Path Can Attack!\n" ) ;
        return 0;
    }
    if ( !CheckNoVio(year + PLUS) )
    {
        printf( "Too Tight Clock Period! \n" ) ;
        return 0;
    }
    HASHTABLE *hptr = new HASHTABLE(16,(unsigned)PathC.size()) ;
    //------------- Read file.cp -------------------------------------------------------------
    string cpname = "./benchmark/" +filename + ".cp" ;
    ReadCpInfo(cpname,1);
    //------------- Cal Original LT ----------------------------------------------------------
    CheckOriLifeTime()  ;
    
    //------------- Variables Declaraion -----------------------------------------------------
    double  bestup   = 100   ;
    double  bestlow  = -100  ;
    int     bestdcc  = 10000 ;
    int     c_sol    = 0     ;
    int     c_nosol  = 0     ;
    double  monteU   = 0     ;
    double  monteL   = 0     ;
    bool *  bestnode = new bool[PathC.size()] ;
    
    //--------------- Main Area ---------------------------------------------------------------
    for( int tryi = 0 ; tryi < trylimit; tryi++ )
    {
        printf( YELLOW "\nRound : %d \n" RESET , tryi ) ;
        //------------- [1] MDS ----------------------------------------------------------------
        if( !ChooseVertexWithGreedyMDS( year, false, hptr )  )
        {
            printf( RED "No Dominate Set! \n" RESET )   ;
            ChooseVertexWithGreedyMDS( year, true , hptr ) ;
            c_nosol++   ;
            continue    ;
        }
        GenerateSAT("./CNF/sat.cnf", year )                   ;
        int oridccs = CallSatAndReadReport(0)           ;
        ChooseVertexWithGreedyMDS( year, true , hptr )  ;
        if( !oridccs )
        {
            if( bestup < 10 && bestlow > 1 ){   printf( RED "Current Best Q = %f ~ %f \n", bestup ,bestlow ) ; }
            printf( RED "No Solution!\n" RESET ) ;
            c_nosol++  ;
            continue   ;
        }
        else    {   printf( GREEN "A Solution Exist\n" RESET ) ;    }
        
           
        //------------- [2] Add Node -------------------------------------------------------------
        //1.It might lead to spare dccs if we add too many nodes.
        for( int i = 0; i < PathC.size(); i++ ){    PathC[i]->SetTried(PathC[i]->Is_Chosen()) ; }
        
        for( int i = 0; i < reftime ; i++ )
        {
            int AddNodeIndex = RefineResult(year,false);
            if( AddNodeIndex < 0 )
            {   break   ;  }
            else
            {
                PathC[AddNodeIndex]->SetChoose(true);
                GenerateSAT("./CNF/sat.cnf", year)        ;
            }
            PathC[AddNodeIndex]->SetTried(true) ;
            int dccs = CallSatAndReadReport(0)  ;
            if( !dccs )
            {
                PathC[AddNodeIndex]->SetChoose(false);
                continue;
            }
            oridccs = dccs;
        }
            
        //------------- [3] Remove Additional DCCs --------------------------------------------------
        GenerateSAT("./CNF/sat.cnf", year )        ;
        system("cp ./CNF/sat.cnf ./CNF/backup.cnf")      ;
        RemoveRDCCs()                        ;
        
        int     dccs  = CallSatAndReadReport(0) ;
        double  upper, lower                ;//Units are "%" not "years".
        if( dccs == 0 || oridccs < dccs )    //After RemoveDCC,if we get NoSol/Poor Sol,
        {                                    //take back the preceding backup file.
            system("cp ./CNF/backup.cnf ./CNF/sat.cnf") ;
        }
        dccs = CallSatAndReadReport(0)           ;
        CalQuality( year, upper, lower, 0/*Q_mode*/ ) ;//Calculate quality.
        
        if( BInv(bestup, bestlow, bestup, bestlow, upper, lower, year,bestdcc,bestdcc,dccs))
        {
            for( int i = 0; i < PathC.size(); i++ ){    bestnode[i] = PathC[i]->Is_Chosen() ;   }
            system("cp ./CNF/sat.cnf ./CNF/best.cnf")   ;
        }
        printf("After Remove Spare DCCs: \n")                ;
        printf("Q = %f ~ %f (此次MDS解)\n", upper ,lower  )   ;
        printf("Q = %f ~ %f (至今最好解)\n", bestup ,bestlow ) ;
        
        //------------- [4] Reverse Current Soltion ---------------------------------------------------
        for( int i = 0; i < reftime ; i++ )
        {
            if( !AnotherSol() ){ break ; }
            dccs = CallSatAndReadReport(0)  ;
            if( !dccs ){ break ;   }
            CalQuality( year, upper, lower, /*Q_mode*/0 );
            if( BInv(bestup, bestlow, bestup, bestlow, upper, lower, year, bestdcc, bestdcc, dccs) )
            {
                system("cp ./CNF/sat.cnf ./CNF/best.cnf")  ;
            }
            printf("After Reversion: \n" )                       ;
            printf("Q = %f ~ %f (此次MDS:解)\n", upper ,lower  )   ;
            printf("Q = %f ~ %f (至今最好解)\n", bestup ,bestlow ) ;
        }
        c_sol++;
    }//for(tryi)
        
    if( bestup > 10 ){
        printf( "NO SOLUTION! \n" );
        return 0 ;
    }
    
    bestup = 100, bestlow = -100, bestdcc = 10000;
    
    //------------- Final Refine -----------------------------------------------------------------
    printf("Final Refinement based on the preceding best solution.\n" )             ;
    for( int i = 0 ; i < PathC.size(); i++ ){   PathC[i]->SetChoose( bestnode[i] )  ;   }
    system("cp ./CNF/best.cnf ./CNF/sat.cnf")       ;
    int dccs = CallSatAndReadReport(0)  ;
    
    //-------- Try Reversing Before The End -------------------------------------------------------
    do{
        double upper, lower;
        CalQuality( year, upper, lower, /*Q_mode*/0 ) ;
        if( BInv( bestup, bestlow, bestup, bestlow, upper, lower, year,bestdcc,bestdcc,dccs) )
        {
            //Monte_CalQuality(year, monteU, monteL)  ;
            system( "cp ./CNF/sat.cnf ./CNF/best.cnf" );
        }
        printf("Q = %f ~ %f \n", upper ,lower  )        ;
        printf("Best Q = %f ~ %f \n", bestup ,bestlow ) ;

        if( !AnotherSol() )
        {   break ;  }
        dccs = CallSatAndReadReport(0) ;
        if( !dccs )
        {   break ;  }
    } while( FINAL-- ) ;
    
    printf("Begin PV-Simulation \n" )                ;
    //------------- Monte -----------------------------------------------------------------------
    //Monte_CalQuality( year, monteU, monteL, Q_mode ) ;//Close it for speeding up tuning arguments
    //RefineResult( year, true )                       ;
    //------------- PV Monte Simulation ---------------------------------------------------------
    PV_Monte_Simulation( PVtimes , year , _vPV, bestup, bestlow  ) ;
    //------------- Release Memory --------------------------------------------------------------
    release( hptr ) ;
    //------------- Show Final Results ----------------------------------------------------------
    printf("\n------------Final Result-----------------------\n") ;
    int BestDccs = CallSatAndReadReport(1)      ;
    int cands = info[1]+info[2]+info[3]-info[4] ;
    RefineResult( year, true )                  ;//設為true才不會return數值.
    /*
    if( monte_s ){
        printf("BEST Q = %f ~ %f (Monte)\n",monteU,monteL)  ;
    }
     */
    printf("BEST Q = %f ~ %f \n", bestup, bestlow )         ;
    printf("Q = %f ~ %f \n", bestup ,bestlow   )            ;
    printf("dccs # = %d \n" , BestDccs )                    ;
    printf("cands # = %d \n" , cands )                      ;
    printf("mine # = %d \n" , (int)info[4] )                ;
    printf("\n");
    //PV_show( )  ;
    return 0;
}

