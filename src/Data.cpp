
#include "Data.h"

#define RED     "\x1b[31m"
#define GRN     "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define RESET   "\x1b[0m"
#define LName  pptr.Gate(0)->GetName().c_str()
#define RName  pptr.Gate(pptr.length()-1)->GetName().c_str()
#define LNameI pptri->Gate(0)->GetName().c_str()
#define RNameI pptri->Gate(pptri->length() - 1)->GetName().c_str()
#define LNameJ pptrj->Gate(0)->GetName().c_str()
#define RNameJ pptrj->Gate(pptrj->length() - 1)->GetName().c_str()
/////////////////////////////////////////////////////////////
extern double arc_thd          ;
extern double year             ;
extern double period           ;
extern struct info *_sInfo     ;
/////////////////////////////////////////////////////////////
extern long int dot_ctr        ;
/////////////////////////////////////////////////////////////
extern int    R_Times          ;
extern int    R_Thre           ;
extern int    L_Times          ;
extern int    L_Thre           ;
extern int    Ref_Times        ;
extern int    Ref_Thre         ;
extern int    Qal_Times        ;
extern int    Qal_Thre         ;
extern int    Q_mode           ;
extern int    FINAL            ;
extern int    trylimit         ;
extern int    reftime          ;
extern int    PVtimes          ;

extern bool   monte_s          ;
extern string filename         ;
/////////////////////////////////////////////////////////////
extern vector <GATE*> _vDCCGate;

string p = "%" ;
/////////////////////////////////////////////////////////////
//      Function                                           //
/////////////////////////////////////////////////////////////
inline double absf(double x)
{
    if (x < 0)  return -x ;
    else        return x  ;
}

inline double maxf(double a, double b)
{
    if (a > b)  return a ;
    else        return b ;
}

inline double minf(double a, double b)
{
    if (a < b)  return a ;
    else        return b ;
}
bool BInv(double &bu, double &bl, double u1, double l1, double u2, double l2,double n,int &dcb,int dc1,int dc2)
{
    if (absf(maxf(absf(u1 - n), absf(l1 - n)) - maxf(absf(u2 - n), absf(l2 - n))) < 0.01)
    {
        if (absf(minf(absf(u1 - n), absf(l1 - n)) - minf(absf(u2 - n), absf(l2 - n))) < 0.01)
        {
            if (dc1<dc2)
            {
                bu = u1 ; bl = l1 ; dcb = dc1;
                return false;
            }
            bu = u2; bl = l2; dcb = dc2;
            return true;
        }
        else if (minf(absf(u1 - n), absf(l1 - n)) < minf(absf(u2 - n), absf(l2 - n)))
        {
            bu = u1; bl = l1; dcb = dc1;
            return false;
        }
        bu = u2; bl = l2; dcb = dc2;
        return true;
    }
    else if (maxf(absf(u1 - n), absf(l1 - n)) < maxf(absf(u2 - n), absf(l2 - n)))
    {
        bu = u1; bl = l1; dcb = dc1;
        return false;
    }
    bu = u2; bl = l2; dcb = dc2;
    return true;
}

void AddNode( )
{
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
        _sInfo->oridccs = dccs;
    }
}
void RemoveAdditionalDCC( bool * bestnode )
{
    GenerateSAT("./CNF/sat.cnf", year )         ;
    system("cp ./CNF/sat.cnf ./CNF/backup.cnf") ;
    RemoveRDCCs()                               ;
    
    _sInfo->dccs  = CallSatAndReadReport(0) ;
    if( _sInfo->dccs == 0 || _sInfo->oridccs < _sInfo->dccs ) //After RemoveDCC,if we get NoSol/Poor Sol, recover the previous backup
    {
        system("cp ./CNF/backup.cnf ./CNF/sat.cnf") ;
    }
    _sInfo->dccs = CallSatAndReadReport(0)           ;
    CalQuality( _sInfo->upper, _sInfo->lower, 0/*Q_mode*/ ) ;//Calculate quality.
    
    if( BInv( _sInfo->bestup, _sInfo->bestlow, _sInfo->bestup, _sInfo->bestlow, _sInfo->upper, _sInfo->lower, year,_sInfo->bestdcc,_sInfo->bestdcc,_sInfo->dccs))
    {
        for( int i = 0; i < PathC.size(); i++ ){ bestnode[i] = PathC[i]->Is_Chosen() ;   }
        system("cp ./CNF/sat.cnf ./CNF/best.cnf")   ;
    }
    printf("After Remove Spare DCCs: \n")                ;
    printf("Q = %f ~ %f (此次MDS解)\n", _sInfo->upper , _sInfo->lower  )  ;
    printf("Q = %f ~ %f (至今最好解)\n", _sInfo->bestup, _sInfo->bestlow ) ;
}
void ReverseSol( )
{
    for( int i = 0; i < reftime ; i++ )
    {
        if( !AnotherSol() ){ break ; }
        _sInfo->dccs = CallSatAndReadReport(0)  ;
        if( !(_sInfo->dccs) ){ break ;   }
        CalQuality( _sInfo->upper, _sInfo->lower, /*Q_mode*/0 );
        if( BInv( _sInfo->bestup, _sInfo->bestlow, _sInfo->bestup, _sInfo->bestlow, _sInfo->upper, _sInfo->lower, year, _sInfo->bestdcc, _sInfo->bestdcc, _sInfo->dccs) )
        {
            system("cp ./CNF/sat.cnf ./CNF/best.cnf")  ;
        }
        printf("After Reversion: \n" )                       ;
        printf("Q = %f ~ %f (此次MDS解)\n", _sInfo->upper , _sInfo->lower  )   ;
        printf("Q = %f ~ %f (至今最好解)\n", _sInfo->bestup , _sInfo->bestlow ) ;
    }
}
void printSetting(  )
{
    printf( CYAN "------------------- Setting --------------------------------\n" RESET ) ;
    printf( CYAN "PLUS  = " GRN "%f\n", PLUS ) ;
    printf( CYAN "TIGHT = " GRN "%f\n", tight ) ;
    printf( CYAN "Final Refimement times = " GRN "%d \n", FINAL ) ;
    
    if( Q_mode != 0 )
    printf( CYAN "PV-Aware Mechanism: " RED "OPEN\n" RESET );
    else
    printf( CYAN "PV-Aware Mechanism: " GRN "CLOSE\n" RESET );
    
    if( Q_mode != 0 )
    {
        printf(  CYAN"Left(-->) |  (<---)Right  \n");
        printf(  GRN "%d" CYAN"/" GRN"%d     " CYAN"|      " GRN"%d" CYAN"/" GRN"%d \n", L_Thre,L_Times,R_Thre,R_Times );
    }
    printf( CYAN "DiGraph Arc Thd = " RED"%f\n", arc_thd ) ;
    printf( CYAN "Vth-pv (Std Deviation) = " GRN"%f\n", PVRange) ;
    if( monte_s )
    printf( CYAN "Monte-Carlo LT Estimation" RED " OPEN \n" )  ;
    else
    printf( CYAN "Monte-Carlo LT Estimation" GRN " CLOSE \n" )  ;
    
    printf( CYAN "Clock Period = " GRN"%f \n", period );
    printf( CYAN "------------------------------------------------------------\n" RESET ) ;
    printf( CYAN "[" BLUE"Candidate" CYAN"]\n" RESET ) ;
    int C = 0, M = 0 ;
    for( int i = 0 ; i < PathC.size() ; i++ )
    {
        if( PathC[i]->GetMine() )
        {   continue ; }
        printf( CYAN"[%d] Len = %d ", C,PathC[i]->length() ) ;
        printf("PDP = %d ", PathC[i]->pldcc ) ;
        printf( BLUE"%s -> %s \n" RESET, PathC[i]->Gate(0)->GetName().c_str() , PathC[i]->Gate(PathC[i]->length()- 1)->GetName().c_str() ) ;
        C++ ;
    }
    printf( CYAN "[" RED"Mine" CYAN"]\n" RESET ) ;
    for( int i = 0 ; i < PathC.size() ; i++ )
    {
        if( PathC[i]->GetMine() )
        {
            printf( CYAN"[%d] Len = %d ", M,PathC[i]->length() ) ;
            printf( RED"%s -> %s\n" RESET, PathC[i]->Gate(0)->GetName().c_str() , PathC[i]->Gate(PathC[i]->length()- 1)->GetName().c_str() ) ;
            M++ ;
        }
    }
    printf( CYAN "------------------------------------------------------------\n" RESET ) ;
}

void ReadParameter( int argc, char* argv[] )
{
    //-------------- Read CMD LINE ------------------------------------------------------------------------
    filename    = argv[1]                       ;
    year        = atof( argv[2] )               ;
    trylimit    = atoi( argv[3] )               ;
    reftime     = atoi( argv[4] )               ;
    PVtimes     = atoi( argv[5] )               ;
    ERROR       = (argc==7)?atof(argv[6]):year*0.1;
    PLUS        = ERROR                         ;
    tight       = 1.000001                      ;
    Q_mode      = 0                             ;
    //-------------- Read Parameter.txt -------------------------------------------------------------------
    fstream file    ;
    string  line    ;
    file.open("Parameter.txt");
    while( getline(file, line) )
    {
        if( line.find("PLUS") != string::npos )
        {
            if( line.find("auto") != string::npos){ PLUS = ERROR ;  }
            else if (line.find("fixed")!=string::npos)
            {
                double f = atof(line.c_str() + 10)  ;
                PLUS = f - year                     ;
            }
            else{   PLUS = atof(line.c_str() + 4)   ;   }
        }
        if( line.find("PV-Aware Open")          != string::npos )    Q_mode    = 3                          ;// --->
        if( line.find("PV-Aware Close")         != string::npos )    Q_mode    = 0                          ;// exclude pv
        if( line.find("PV_Aware_thd_right")     != string::npos )    R_Thre    = atoi(line.c_str() + 18 )   ;
        if( line.find("PV_Aware_tim_right")     != string::npos )    R_Times   = atoi(line.c_str() + 18 )   ;
        if( line.find("PV_Aware_thd_left")      != string::npos  )   L_Thre    = atoi(line.c_str() + 17 )   ;
        if( line.find("PV_Aware_tim_left")      != string::npos  )   L_Times   = atoi(line.c_str() + 17 )   ;
        if( line.find("Vth_pv")                 != string::npos )    PVRange   = atof(line.c_str() + 6 )    ;
        if( line.find("TIGHT")                  != string::npos )    tight     = atof(line.c_str() + 5 )    ;
        if( line.find("FINAL")                  != string::npos )    FINAL     = atof(line.c_str() + 5 )    ;
        if( line.find("Refine-Tim")             != string::npos )    Ref_Times = atoi(line.c_str() + 10 )   ;
        if( line.find("Refine-Thd")             != string::npos )    Ref_Thre  = atoi(line.c_str() + 10 )   ;
        if( line.find("Quality-Tim")            != string::npos )    Qal_Times = atoi(line.c_str() + 11 )   ;
        if( line.find("Quality-Thd")            != string::npos )    Qal_Thre  = atoi(line.c_str() + 11 )   ;
        if( line.find("thershold")              != string::npos )    arc_thd   = atof(line.c_str() + 9 )   ;
        if( line.find("MONTE YES")              != string::npos )    monte_s   = true                       ;
    }
}

////////////////////////////////////////////////////////////////////////////////////
//              Release Memory                                                    //
////////////////////////////////////////////////////////////////////////////////////
void release( HASHTABLE *hptr )
{
    for( int i = 0; i < PathC.size() ; i++ )
    {
        delete EdgeA[i] ;
        delete EdgeB[i] ;
        delete cor[i]   ;
        delete ser[i]   ;
    }
    delete hptr  ;
    delete EdgeA ;
    delete EdgeB ;
    delete cor   ;
    delete ser   ;
}



void Region( double &year_lower, double &year_upper, double &L, double &R )
{
    static int R1 = 0, R2 = 0, R3 = 0, R4 = 0, R5 = 0, R6 = 0 ;
    
    if( year_lower < L )
    {
        if( year_upper < L )        R1++ ;
        else if( year_upper < R )   R2++ ;
        else                        R3++ ;
    }
    else if( year_lower < R )
    {
        if(  year_upper < R )       R6++ ;
        else                        R4++ ;
    }
    else                            R5++ ;
    printf( "R1 = %d, R2 = %d, R3 = %d, R4 = %d, R5 = %d, R6 = %d\n\n", R1, R2, R3, R4, R5, R6) ;
}
////////////////////////////////////////////////////////////////////////////////////
//              PV Simulator                                                      //
////////////////////////////////////////////////////////////////////////////////////

bool compare( struct PVdata* A, struct PVdata*B ){  return ( A->gdist() < B->gdist() ) ; }

void PV_Monte_Simulation(  double bu, double bl  )
{
    vector< struct PVdata* > _vPV           ;
    FILE *fupper100 = fopen("./quality/Q_upper100.txt","w+t") ;
    FILE *flower100 = fopen("./quality/Q_lower100.txt","w+t") ;
    
    double PV_monteU = 0, PV_monteL = 0 ;
    double L = year-ERROR, R = year+ERROR ;
    chrono::steady_clock::time_point starttime, endtime, pre_time, ed_time ;
    chrono::duration<double>  PVSeedTime, TotalPVSeedTime ;
    pre_time = chrono::steady_clock::now();
    //-----------------------------------------------------------------------------------
    for( int i = 1 ; i <= PVtimes ; i++ )
    {
        
        PV_monteU = PV_monteL = 0.0              ;
        printf( YELLOW "[ %d/%d Instance Simulation ]\n" RESET , i, PVtimes )   ;
        starttime = chrono::steady_clock::now();
        Monte_PVCalQuality( PV_monteU, PV_monteL )      ;
        endtime = chrono::steady_clock::now();
        PVSeedTime = chrono::duration_cast<chrono::duration<double>>(endtime - starttime);
        printf( "Q(PV) : ") ;
        if( PV_monteU < year - ERROR ) printf( RED )  ;
        else                           printf( RESET );
        printf("%f ~ ", PV_monteU );
        if( PV_monteL > year + ERROR ) printf( GRN )  ;
        else                           printf( RESET );
        printf("%f\n", PV_monteL );
        printf( RESET );
        cout << "PV Seed Time = " << CYAN << PVSeedTime.count() << RESET<< endl  ;
        Region( PV_monteU, PV_monteL, L, R ) ;
        fprintf( fupper100, "%f\n", PV_monteU )          ;
        fprintf( flower100, "%f\n", PV_monteL )          ;
    }
    ed_time = chrono::steady_clock::now();
    TotalPVSeedTime = chrono::duration_cast<chrono::duration<double>>(ed_time - pre_time) ;
    printf( CYAN"-------------- PV Summary------------------------------------\n") ;
    cout << CYAN << "Total Seed #    = " << GRN << PVtimes << RESET << endl ;
    cout << CYAN << "Total Seed Time = " << GRN << TotalPVSeedTime.count() << RESET << endl ;
}
////////////////////////////////////////////////////////////////////////////////////
//              PV Simulator - Instance Generator                                 //
////////////////////////////////////////////////////////////////////////////////////
void GeneratePVCkt()
{
    for( int i = 0 ; i < PathR.size() ; i++ )
    {
        /*
        PathR[i].SetPVMine(false) ;
        PathR[i].SetPVCand(false) ;
        PathR[i].SetPVSafe(true)  ;
         */
        for( int j = 1 ; j < PathR[i].gTiming()->size() ; j++ )
        {
            double U = rand() / (double)RAND_MAX                    ;
            double V = rand() / (double)RAND_MAX                    ;
            double Z = sqrt(-2 * log(U))*cos(2 * 3.14159265354*V)   ;
            PathR[i].gTiming(j)->setVth_pv( Z*PVRange )                ;//0.01 mean 10mv
        }
    }
}
////////////////////////////////////////////////////////////////////////////////////
//              PV Simulator - Quality Calculator                                 //
////////////////////////////////////////////////////////////////////////////////////
double Monte_PVCalQuality( double &up, double &low )
{
    up = 10.0; low = 0.0            ;
    int TryT = 3000 / PathC.size()  ;
    if( TryT < 30 ) TryT = 30       ;
    map< double , worse* > worse    ;
    vector< double> monte           ;
    monte.clear( )                  ;
    /*
    int Dots100_ctr  = 0            ;
    int Dots95_ctr  = 0             ;
    */
    //-------------------- Generate a New Instance with PV --------------------------
    GeneratePVCkt( )                            ;
    
    //--------------------- Analyze the New Instance --------------------------------
    double U = 0, V = 0, Z = 0 ;
    double st = 0, ed = 0, mid = 0, lt = 0 ;
    PATH * pptr = NULL ;
    double AgR_AtoB      = 0 ;
    double AgR_A_Wst_mid = 0 ;
    double AgR_A_Wst_ten = 0 ;
    double AgR_B_RL      = 0 ;
    double AgR_B_MC      = 0 ;
    
    for( int i = 0; i < PathC.size() ; i++ )//Path A
    {
        if( !PathC[i]->GetCand() )  continue ;
        AgR_A_Wst_ten = CalPathAginRateWithPV( PathC[i], 10  ) ;
        
        for( int tt = 0; tt < TryT; tt++ )
        {
            lt = 10000  ;
            AgR_AtoB = 0;
            pptr = NULL ;
        
            for( int j = 0; j < PathC.size(); j++ )//Path B
            {
                if( EdgeA[i][j] > 9999 )  continue;
                st  = 1  ;
                ed  = 10 ;
                mid = 0  ;
                U = rand() / (double)RAND_MAX;
                V = rand() / (double)RAND_MAX;
                Z = sqrt(-2 * log(U))*cos(2 * 3.14159265354*V) ;
    
                //#####-------Binary Search---------------------------------------------
                while( ed - st > 0.001 )
                {
                    mid = (st + ed) / 2;
                    AgR_A_Wst_mid = CalPathAginRateWithPV( PathC[i], mid ) ;//Paht A worst-case aging rate
                    AgR_B_RL  = CalPreAging( AgR_A_Wst_mid, i, j, mid ) ;//Path B aging rate by regression line(RL)
                    AgR_B_MC = Z*( ser[i][j] * (1 + AgR_A_Wst_mid)/(1 + AgR_A_Wst_ten) ) + AgR_B_RL;//Path B aging rate by monte-carlo(MC)
                    
                    if( AgR_B_MC > AgR_A_Wst_mid )
                        AgR_B_MC = AgR_A_Wst_mid ;
                    if( Vio_Check( PathC[j], mid, AgR_B_MC , 0/*Exclude PV*/, 0, 0 ) )
                        st = mid;
                    else
                        ed = mid;
                }
                if( mid < lt ){
                    lt = mid            ;
                    pptr = PathC[j]     ;
                    AgR_AtoB = AgR_B_MC ;
                }
            }
            monte.push_back( lt );
        }//for(tt)
    }//for(_vPathC[i])
    
    sort( monte.begin(), monte.end() )  ;
    int front = 0                       ;
    int back = (int)monte.size() - 1    ;
    up = monte[front]; low = monte[back];
    
    //#------------ Remove Remotest 5% Dots --------------------------------------------
    while( front + monte.size() - 1 - back <= monte.size()/20 )
    {
        if( absff(monte[front] - (double)year) > absff(monte[back] - (double)year ) )
            up = monte[++front];
        else
            low = monte[--back];
    }
    worse.clear()  ;
    
    return 0.0;
}

int shortlistsize()
{
    int ctr = 0 ;
    for( auto pptr : PathR ){ if( pptr.Is_Chosen() ) ctr++ ; }
    return ctr ;
}


