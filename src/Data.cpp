
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


string p = "%" ;
/////////////////////////////////////////////////////////////
//      Function                                           //
/////////////////////////////////////////////////////////////
double CIRCUIT::calConvergentVth( double dc, double exp )
{
    return 0.0039/2*( pow( dc*(convergent_year)*(31536000), Exp )) ;
}
double CIRCUIT::calSv( double dc, double VthOffset, double VthFin  )
{
    if( VthOffset == 0 ) return 0 ;
    //The func refers to "hi_n_low_buffer.py"
    double Right   = VthFin - VthOffset                  ;
    double Time    = dc*( convergent_year )*( 31536000 ) ;
    double C       = 0.0039/2*( pow( Time, this->Exp ) ) ;
    return -( Right/(C) - 1 )/(VthOffset)                ;
}


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
bool CIRCUIT::BInv(double &bu, double &bl, double u1, double l1, double u2, double l2,double n,int &dcb,int dc1,int dc2)
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


void CIRCUIT::ReverseSol( )
{
    /*
    printf( YELLOW "---------------------------------------------\n" RESET );
    printf( YELLOW"[3] [Reversing Prior Solution]\n" RESET )   ;
    for( int i = 0; i < reftime ; i++ )
    {
        if( !AnotherSol() ){ break ; }
        
        //_sInfo->dccs = CallSatAndReadReport(0)  ;
        if( !(CallSatAndReadReport(0)) ){ break ;   }
        this->CalQuality( upper, lower,0 );
        if( BInv( bestup, bestlow, bestup, bestlow, upper, lower, year, bestdcc, bestdcc, dccs) )
        {
            system("cp ./CNF/sat.cnf ./CNF/best.cnf")  ;
        }
        printf( YELLOW "---------------------------------------------\n" RESET );
        printf( YELLOW "[4] [Final Quality] \n" RESET );
        printf("   Q = %f ~ %f (此次MDS解)\n", upper , lower  )   ;
        printf("   Q = %f ~ %f (至今最好解)\n", bestup , bestlow ) ;
    }*/
     
}
void CIRCUIT::printSetting(  )
{
    printf( CYAN "------------------- Setting --------------------------------\n" RESET ) ;
    printf( CYAN "Benchmark  = " GRN "%s\n", filename.c_str() ) ;
    printf( CYAN "Year  = " GRN "%f\n", year  ) ;
    printf( CYAN "C.Y.  = " GRN "%f\n", convergent_year  ) ;
    printf( CYAN "ERROR = " GRN "%f\n", ERROR ) ;
    printf( CYAN "PLUS  = " GRN "%f\n", PLUS  ) ;
    printf( CYAN "TIGHT = " GRN "%f\n", tight ) ;
    printf( CYAN "Tc Margin  = " GRN "%f\n", tc_mgn  ) ;
    printf( CYAN "A     = " GRN "%f\n", this->A  ) ;
    printf( CYAN "Alpha = " GRN "%f\n", this->alpha  ) ;
    printf( CYAN "Exp   = " GRN "%f\n", this->Exp  ) ;
    printf( CYAN "Final Refimement times = " GRN "%d \n", FINAL ) ;
    
    /*
    if( Q_mode != 0 )
    printf( CYAN "PV-Aware Mechanism: " RED "OPEN\n" RESET );
    else
    printf( CYAN "PV-Aware Mechanism: " GRN "CLOSE\n" RESET );
    
    if( Q_mode != 0 )
    {
        printf(  CYAN"Left(-->) |  (<---)Right  \n");
        printf(  GRN "%d" CYAN"/" GRN"%d     " CYAN"|      " GRN"%d" CYAN"/" GRN"%d \n", L_Thre,L_Times,R_Thre,R_Times );
    }
     */
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
    for( int i = 0 ; i < getPathCand().size() ; i++ )
    {
        if( getPathCand().at(i)->GetMine() )
        {   continue ; }
        printf( CYAN"[%d] Len = %d ", C,getPathCand().at(i)->length() ) ;
        printf("PDP = %d ", getPathCand().at(i)->pldcc ) ;
        printf( BLUE"%s -> %s \n" RESET, getPathCand().at(i)->Gate(0)->GetName().c_str() , getPathCand().at(i)->Gate(getPathCand().at(i)->length()- 1)->GetName().c_str() ) ;
        C++ ;
    }
    printf( CYAN "[" RED"Mine" CYAN"]\n" RESET ) ;
    for( int i = 0 ; i < getPathCand().size() ; i++ )
    {
        if( getPathCand().at(i)->GetMine() )
        {
            printf( CYAN"[%d] Len = %d ", M,getPathCand().at(i)->length() ) ;
            printf( RED"%s -> %s\n" RESET, getPathCand().at(i)->Gate(0)->GetName().c_str() , getPathCand().at(i)->Gate(getPathCand().at(i)->length()- 1)->GetName().c_str() ) ;
            M++ ;
        }
    }
    printf( CYAN "------------------------------------------------------------\n" RESET ) ;
}

bool CIRCUIT::ReadParameter( int argc, char* argv[], string &message )
{
    //-------------- Read CMD LINE ------------------------------------------------------------------------
    filename    = argv[1]                       ;
    PLUS        = ERROR                         ;
    tight       = 1.000001                      ;
    Q_mode      = 0                             ;
    //-------------- Read Parameter.txt -------------------------------------------------------------------
    fstream file    ;
    string  line    ;
    file.open("./parameter/Parameter.txt");
    while( getline(file, line) )
    {
        if( line.find("Vth_SD(PV)")             != string::npos )    this->PVRange   = atof(line.c_str() + 10 )    ;
        if( line.find("TIGHT")                  != string::npos )    this->tight     = atof(line.c_str() + 5 )    ;
        if( line.find("MDS_Times")              != string::npos )    this->trylimit  = atoi(line.c_str() + 9 )    ;
        if( line.find("Instance_Ctr")           != string::npos )    this->PVtimes   = atoi(line.c_str() + 12 )    ;
        if( line.find("FINAL")                  != string::npos )    this->FINAL     = atof(line.c_str() + 5 )    ;
        if( line.find("Arc-Thd")                != string::npos )    this->arc_thd   = atof(line.c_str() + 7 )   ;
        if( line.find("Attack_yr")              != string::npos )    this->year      = atof(line.c_str() + 9 )   ;
        if( line.find("Error_yr")               != string::npos )    this->ERROR     = atof(line.c_str() + 8 )   ;
        if( line.find("MONTE YES")              != string::npos )    this->monte_s   = true                       ;
        if( line.find("Convergent_Year")        != string::npos )    this->convergent_year = atof(line.c_str() + 15 )        ;
        if( line.find("Exp")                    != string::npos )    this->Exp       = atof(line.c_str() + 3 )        ;
        if( line.find("A_C")                      != string::npos )  this->A         = atof(line.c_str() + 3 )        ;
        if( line.find("Alpha")                  != string::npos )    this->alpha     = atof(line.c_str() + 5 )        ;
        if( line.find("PLUS") != string::npos )
        {
            if( line.find("auto") != string::npos){ PLUS = ERROR ;  }
            else if (line.find("fixed")!=string::npos)
            {
                double f = atof(line.c_str() + 10)  ;
                cout << "f " << f << endl ;
                PLUS = f - year                     ;
            }
            else{   PLUS = atof(line.c_str() + 4)   ;   }
        }
    }
    
    this->VTH_CONVGNT[0] = this->calConvergentVth( 0.2, Exp ) ;//20% DCC
    this->VTH_CONVGNT[1] = this->calConvergentVth( 0.4, Exp ) ;//40% DCC
    this->VTH_CONVGNT[2] = this->calConvergentVth( 0.5, Exp ) ;//50% DCC/None
    this->VTH_CONVGNT[3] = this->calConvergentVth( 0.8, Exp ) ;//80% DCC
    this->Sv[0]          = this->calSv( 0.2, PVRange, VTH_CONVGNT[0] ) ;//20% DCC
    this->Sv[1]          = this->calSv( 0.4, PVRange, VTH_CONVGNT[1] ) ;//20% DCC
    this->Sv[2]          = this->calSv( 0.5, PVRange, VTH_CONVGNT[2] ) ;//20% DCC
    this->Sv[3]          = this->calSv( 0.8, PVRange, VTH_CONVGNT[3] ) ;//20% DCC
    
    return true ;
}

////////////////////////////////////////////////////////////////////////////////////
//              Release Memory                                                    //
////////////////////////////////////////////////////////////////////////////////////
void CIRCUIT::release( HASHTABLE *hptr )
{
    for( int i = 0; i < getPathCand().size() ; i++ )
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
        if( year_upper < L )       {   R1++ ; printf("R1++\n"); }
        else if( year_upper < R )  {   R2++ ; printf("R2++\n"); }
        else                       {   R3++ ; printf("R3++\n"); } ;
    }
    else if( year_lower < R )
    {
        if(  year_upper < R )      {   R6++ ; printf("R6++\n"); }
        else                       {   R4++ ; printf("R4++\n"); }
    }
    else                           {   R5++ ; printf("R5++\n"); }
    printf( "R3 = %d, R4 = %d, R5 = %d\n", R3, R4, R5 ) ;
    printf( "R2 = %d, R6 = %d\n", R2, R6 ) ;
    printf( "R1 = %d\n\n", R1 ) ;
}
////////////////////////////////////////////////////////////////////////////////////
//              PV Simulator                                                      //
////////////////////////////////////////////////////////////////////////////////////

//bool compare( struct PVdata* A, struct PVdata*B ){  return ( A->gdist() < B->gdist() ) ; }

void CIRCUIT::PV_Monte_Simulation(  )
{
    vector< struct PVdata* > _vPV           ;
    FILE *foutput = fopen("./quality/LT.txt","w+t") ;
    
    double PV_monteU = 0, PV_monteL = 0 ;
    //double L = year-ERROR, R = year+ERROR ;
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
        if( PV_monteL < year - ERROR ) printf( RED )  ;
        else                           printf( RESET );
        printf("%f ~ ", PV_monteL );
        if( PV_monteU > year + ERROR ) printf( GRN )  ;
        else                           printf( RESET );
        printf("%f\n", PV_monteU );
        printf( RESET );
        cout << "PV Seed Time = " << CYAN << PVSeedTime.count() << RESET<< endl  ;
        
        //Region( PV_monteU, PV_monteL, L, R ) ;
        fprintf( foutput, "%f %f\n", PV_monteL, PV_monteU )          ;
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
void CIRCUIT::GeneratePVCkt()
{
    for( int i = 0 ; i < this->getPathALL().size() ; i++ )
    {
        /*
        PathR[i].SetPVMine(false) ;
        PathR[i].SetPVCand(false) ;
        PathR[i].SetPVSafe(true)  ;
         */
        for( int j = 1 ; j < this->getPathALL().at(i).gTiming()->size() ; j++ )
        {
            double U = rand() / (double)RAND_MAX                    ;
            double V = rand() / (double)RAND_MAX                    ;
            double Z = sqrt(-2 * log(U))*cos(2 * 3.14159265354*V)   ;
            this->getPathALL().at(i).gTiming(j)->setVth_pv( Z*PVRange )                ;//0.01 mean 10mv
        }
    }
}
////////////////////////////////////////////////////////////////////////////////////
//              PV Simulator - Quality Calculator                                 //
////////////////////////////////////////////////////////////////////////////////////
double CIRCUIT::Monte_PVCalQuality( double &low, double &up )
{
    up = 10.0; low = 0.0            ;
    int TryT = 3000 / getPathCand().size()  ;
    if( TryT < 30 ) TryT = 30       ;
    
    vector< double> monte           ;
    monte.clear( )                  ;
    //-------------------- Generate a New Instance with PV --------------------------
    this->GeneratePVCkt( )          ;
    
    //--------------------- Analyze the New Instance --------------------------------
    double U = 0, V = 0, Z = 0 ;
    double st = 0, ed = 0, mid = 0, lt = 0 ;
    PATH * pptr = NULL ;
    double AgR_AtoB      = 0 ;
    double AgR_A_Wst_mid = 0 ;
    double AgR_A_Wst_ten = 0 ;
    double AgR_B_RL      = 0 ;
    double AgR_B_MC      = 0 ;
    
    for( int i = 0; i < getPathCand().size() ; i++ )//Path A
    {
        if( !getPathCand().at(i)->GetCand() )  continue ;
        AgR_A_Wst_ten = CalPathAginRateWithPV( getPathCand().at(i), 10  ) ;
        
        for( int tt = 0; tt < TryT; tt++ )
        {
            lt = 10000  ;
            AgR_AtoB = 0;
            pptr = NULL ;
        
            for( int j = 0; j < getPathCand().size(); j++ )//Path B
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
                    AgR_A_Wst_mid = CalPathAginRateWithPV( getPathCand().at(i), mid ) ;//Paht A worst-case aging rate
                    AgR_B_RL      = CalPreAging( AgR_A_Wst_mid, i, j, mid ) ;//Path B aging rate by regression line(RL)
                    AgR_B_MC      = Z*( ser[i][j] * (1 + AgR_A_Wst_mid)/(1 + AgR_A_Wst_ten) ) + AgR_B_RL;//Path B aging rate by monte-carlo(MC)
                    
                    if( AgR_B_MC > AgR_A_Wst_mid )
                        AgR_B_MC = AgR_A_Wst_mid ;
                    if( Vio_Check( getPathCand().at(j), mid, AgR_B_MC, 0 ) )
                        st = mid;
                    else
                        ed = mid;
                }
                if( mid < lt ){
                    lt = mid            ;
                    pptr = getPathCand().at(j)     ;
                    AgR_AtoB = AgR_B_MC ;
                }
            }
            monte.push_back( lt );
        }//for( tt )
    }//for( getPathCand().at(i) )
    
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
    
    return 0.0;
}

int CIRCUIT::shortlistsize()
{
    int ctr = 0 ;
    for( auto pptr : this->getPathALL() ){ if( pptr.Is_Chosen() ) ctr++ ; }
    return ctr ;
}


