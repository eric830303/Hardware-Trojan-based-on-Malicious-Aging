
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
extern double _pvPeriod        ;
extern double **_pEdgeA        ;
extern double **_pEdgeB        ;
extern double **_pcor          ;
extern double **_pser          ;
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
extern int    pv_times         ;
/////////////////////////////////////////////////////////////
extern vector <GATE*> _vDCCGate;
int GPre_Diver_Dot = 0         ;
int GPos_Diver_Dot = 0         ;
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

void ReadParameter( double &PLUS , double &tight , int &FINAL , bool &monte_s , int year , int &TotalTimes , int &Threshold, double &PV )
{
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
        if( line.find("Global_Times")  != string::npos )    TotalTimes= atoi(line.c_str() + 12 )   ;
        if( line.find("Global_Th")     != string::npos )    Threshold = atoi(line.c_str() + 9 )    ;
        if( line.find("PVRange")       != string::npos )    PV        = atof(line.c_str() + 7 )    ;
        if( line.find("R_Threshold")   != string::npos )    R_Thre    = atoi(line.c_str() + 11 )   ;
        if( line.find("R_RunTime")     != string::npos )    R_Times   = atoi(line.c_str() + 9 )    ;
        if( line.find("L_Threshold")   != string::npos )    L_Thre    = atoi(line.c_str() + 11 )   ;
        if( line.find("L_RunTime")     != string::npos )    L_Times   = atoi(line.c_str() + 9 )    ;
        if( line.find("TIGHT")         != string::npos )    tight     = atof(line.c_str() + 5 )    ;
        if( line.find("FINAL")         != string::npos )    FINAL     = atof(line.c_str() + 5 )    ;
        if( line.find("Ref_Times")     != string::npos )    Ref_Times = atoi(line.c_str() + 9 )    ;
        if( line.find("Ref_Threshold") != string::npos )    Ref_Thre  = atoi(line.c_str() + 13 )   ;
        if( line.find("Qual_Times")    != string::npos )    Qal_Times = atoi(line.c_str() + 10 )   ;
        if( line.find("Qual_Threshold")!= string::npos )    Qal_Thre  = atoi(line.c_str() + 14 )   ;
        if( line.find("MONTE YES")     != string::npos )    monte_s   = true                       ;
        if( line.find("Q_Monte Open")  != string::npos )    Q_mode    = 3                          ;// --->
        if( line.find("Q_Monte Close") != string::npos )    Q_mode    = 0                          ;// exclude pv
    }
    
    printf( "PLUS = %f ; TIGHT = %f ; Final Refine = %d \n" , PLUS , tight , FINAL ) ;
    printf( "Quality Mode = " RED "%d \n" RESET, Q_mode )                            ;
    printf( "Threshold = %d / TotalTimes = %d \n",TotalTimes,Threshold )    ;
    printf( "L: %d/%d , R: %d/%d \n",L_Thre,L_Times,R_Thre,R_Times )        ;
    printf( "Monte-Carlo is " )                                             ;
    if( monte_s )   printf( MAGENTA "OPEN\n"  RESET )  ;
    else            printf( MAGENTA "CLOSE\n" RESET )  ;
}

////////////////////////////////////////////////////////////////////////////////////
//              Show The Change of Role of Path After PV                          //
////////////////////////////////////////////////////////////////////////////////////
void PrintPath( PATH &pptr )
{
    if( pptr.GetCand()   )      printf("OC ")       ;
    if( pptr.Is_Chosen() )      printf("OS ")       ;
    if( pptr.IsSafe()    )      printf("OF ")       ;
    if( pptr.GetMine()   )      printf("OM ")       ;
    //-----------------------------------------------------------------------------
    if( pptr.GetPVCand() )      printf("PC ")       ;
    if( pptr.GetPVSafe() )      printf("PF ")       ;
    if( pptr.GetPVMine() )      printf("PM ")       ;
}
void PrintPath( PATH *pptr )
{
    if( pptr->GetCand()   )     printf("OC ")       ;
    if( pptr->Is_Chosen() )     printf("OS ")       ;
    if( pptr->IsSafe()    )     printf("OF ")       ;
    if( pptr->GetMine()   )     printf("OM ")       ;
    //-----------------------------------------------------------------------------
    if( pptr->GetPVCand() )     printf("PC ")       ;
    if( pptr->GetPVSafe() )     printf("PF ")       ;
    if( pptr->GetPVMine() )     printf("PM ")       ;
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

////////////////////////////////////////////////////////////////////////////////////
//              ReDefine PV Circuit                                               //
////////////////////////////////////////////////////////////////////////////////////
void DefCandMineSafe( double year,double margin,bool flag,double PLUS )//CheckPathAttack
{
    double OriginalE = ERROR ;
    _pvPeriod = 0.0 ;
    int mode = 1    ;
    //----------------------------Do Every Path(Year: year+PLUS )--------------------------------//
    for( int i = 0; i < PathR.size(); i++ )
    {
        PATH* pptr = &PathR[i]          ;
        GATE* edptr = pptr->Gate(pptr->length() - 1)    ;
        GATE* stptr = pptr->Gate(0)     ;
        
        //-------------------------Non-Aging Timing----------------------------------------------//
        double clks = pptr->GetCTH()    ;
        double clkt = pptr->GetCTE()    ;
        double Tcq = pptr->Out_time(0) - pptr->In_time(0) ;
        
        //-------------------------Plus Aging Timing of each Buffer on clock path----------------//
        //Timing(right): clock --> FF(end) ,clkt'(modfied) += clkt(original) + aging_timing(no DCC)
        for( int i = 0; i < edptr->ClockLength(); i++ )
        {
            double delay = edptr->GetClockPath(i)->GetOutTime() - edptr->GetClockPath(i)->GetInTime() ;
            clkt += ( delay )*AgingRate(DCC_NONE, year + PLUS) ;
        }
        //Timing(left): clock --> FF(head),clks'(modfied) += clks(original) + aging_timing(no DCC)
        for (int i = 0; i < stptr->ClockLength(); i++)
        {
            double delay = stptr->GetClockPath(i)->GetOutTime() - stptr->GetClockPath(i)->GetInTime() ;
            clks += ( delay )*AgingRate(DCC_NONE, year + PLUS)  ;
        }
        //-------------------------FF Timing with Aging------------------------------------------//
        if( stptr->GetType() != "PI" )//PI是wire時，沒FF，自然沒Tcq
        {
            Tcq *= (1.0 + AgingRate(FF, static_cast<double>(year + PLUS)));
        }
        
        //-------------------------Period Timing with Aging---------------------------------------//
        double Dij =  ( pptr->In_time(pptr->length() - 1) - pptr->Out_time(0) ) ;//沒老化,沒PV
        for( int i = 1 ; i < pptr->length()-1 ; i++ )
        {
            double Delta = ( pptr->Out_time(i) - pptr->In_time(i) )*( pptr->PV_time(i) ) ;
            Dij += Delta ;
        }
        double pp = ( 1 + AgingRate(WORST, static_cast<double>(year + PLUS)))*( Dij ) + Tcq + (clks - clkt) + pptr->GetST();
        pp *= margin ;//margin = 1.000001吧
        if( pp > _pvPeriod )//找出最大值，作為整體電路的period
        {
            _pvPeriod = pp;
        }
    }
    
    if( flag ){ cout << "Instance(PV)'s Clock Period = " << _pvPeriod << endl ; }
    
    for( int i = 0 ; i < PathR.size() ; i++ )
    {
        PATH* pptr = &PathR[i]  ;
        pptr->SetPVAtk(false)   ;
        pptr->SetPVSafe(true)   ;
        GATE* stptr = pptr->Gate(0);
        GATE* edptr = pptr->Gate(pptr->length() - 1);
        int lst = stptr->ClockLength()  ;//length of clockpath(head)
        int led = edptr->ClockLength()  ;//length of clockpath(end)
        int branch = 1                  ;//1(before 2/7)
        
        if( stptr->GetType() == "PI" )
        {
            for( int j = 0 ; j < led ; j++ )//測end端clock path各個位置是否可放DCC?
            {
                for( int x = 0 /*1*/; x <= 3; x++ )
                {
                    auto mapitr = pptr->_mapdcc.find( tuple<int,int,AGT,AGT>(0,j,DCC_NONE,(AGT)x) ) ;
                    assert( mapitr != pptr->_mapdcc.end() ) ;
                    
                    if( !Vio_Check(pptr, 0, j, DCC_NONE, (AGT)x,year+OriginalE,mode,0,0 ))//是否n+e年以內fail?
                    {
                        pptr->SetPVSafe(false)  ;
                        if( Vio_Check(pptr, 0, j, DCC_NONE, (AGT)x, year - OriginalE , mode,0,0 ))
                        {
                            pptr->SetPVAtk(true)          ;
                            mapitr->second->pv_midd(true) ;
                        }
                        else{
                            mapitr->second->pv_pref(true) ;
                        }
                    }
                    else{
                        mapitr->second->pv_posf(true) ;
                    }
                }
            }
        }
        else if( edptr->GetType() == "PO" )//End Gate = "Primary Output"
        {
            for( int j = 0 ; j < lst ; j++ )//測head端clock path各個位置是否可放DCC?
            {
                for( int x = 0 ; x <= 3; x++ )
                {
                    auto mapitr = pptr->_mapdcc.find( tuple<int,int,AGT,AGT>(j,0,(AGT)x,DCC_NONE) ) ;
                    assert( mapitr != pptr->_mapdcc.end() ) ;
                    
                    if( !Vio_Check(pptr, j, 0, (AGT)x, DCC_NONE, year + OriginalE , mode,0,0 ) )
                    {
                        pptr->SetPVSafe(false) ;
                        if( Vio_Check(pptr, j, 0, (AGT)x, DCC_NONE, year - OriginalE , mode,0,0 ) )
                        {
                            pptr->SetPVAtk(true)          ;
                            mapitr->second->pv_midd(true) ;
                        }
                        else{
                            mapitr->second->pv_pref(true) ;
                        }
                    }
                    else{
                        mapitr->second->pv_posf(true) ;
                    }
                }
            }
        }
        else//Path:FF~FF
        {
            while( branch < lst /*length start*/ && branch < led /*length end*/ )
            {
                if( stptr->GetClockPath(branch) != edptr->GetClockPath(branch)) break ;
                for( int x = 0 ; x <= 3 ; x++ )
                {
                    auto mapitr = pptr->_mapdcc.find( tuple<int,int,AGT,AGT>(branch,branch,(AGT)x,(AGT)x) ) ;
                    assert( mapitr != pptr->_mapdcc.end() ) ;
                    
                    if( !Vio_Check(pptr, branch, branch, (AGT)x, (AGT)x, year + OriginalE , mode,0,0 ) )
                    {
                        pptr->SetPVSafe(false)         ;
                        mapitr->second->pv_posf(false) ;
                        if( Vio_Check(pptr, branch, branch, (AGT)x, (AGT)x, year - OriginalE , mode,0,0 ))
                        {
                            pptr->SetPVAtk(true)          ;
                            mapitr->second->pv_midd(true) ;
                        }
                        else{
                            mapitr->second->pv_pref(true) ;
                        }
                    }
                    else{
                        mapitr->second->pv_posf(true) ;
                    }
                }
                branch++;
            }//while( branch < lst && branch < led )
            
            for( int j = branch ; j < lst; j++ )
            {
                for( int k = branch ; k < led; k++)
                {
                    for( int x = 0 ; x <= 3; x++)
                    {
                        for( int y = 0 ; y <= 3; y++)
                        {
                            auto mapitr = pptr->_mapdcc.find( tuple<int,int,AGT,AGT>(j,k,(AGT)x,(AGT)y) ) ;
                            assert( mapitr != pptr->_mapdcc.end() ) ;
                            
                            if( !Vio_Check(pptr,j,k,(AGT)x,(AGT)y,year+OriginalE , mode,0,0 ))
                            {
                                pptr->SetPVSafe(false)      ;
                                if( Vio_Check(pptr,j,k,(AGINGTYPE)x,(AGINGTYPE)y,year-OriginalE , mode,0,0 ) )
                                {
                                    pptr->SetPVAtk(true)          ;
                                    mapitr->second->pv_midd(true) ;
                                }
                                else
                                    mapitr->second->pv_pref(true) ;
                            }
                            else
                                mapitr->second->pv_posf(true) ;
                        }
                    }//for (int x = 0; x < 3; x++)
                }//for (int k = branch; k < led; k++)
            }//for (int j = branch; j < lst; j++)
        }//if-else
    }//for (int i = 0; i < PathR.size(); i++)
    
    int aa = 0 ;//PI#
    int bb = 0 ;//PO#
    int cc = 0 ;//Cand#+Mine#
    int dd = 0 ;//Mine#
    
    for( unsigned i = 0 ; i < PathR.size(); i++ )
    {
        PATH* pptr = &PathR[i]          ;
        GATE* stptr = pptr->Gate(0)     ;
        GATE* edptr = pptr->Gate(pptr->length() - 1)    ;
        pptr->SetPVCand(false)          ;//Initialization
        pptr->SetPVMine(false)          ;//Initialization
        if( !pptr->GetPVSafe() )
        {
            if( stptr->GetType() == "PI" )      aa++    ;
            else if( edptr->GetType() == "PO")  bb++    ;
            else                                cc++    ;
            
            _vPathC.push_back(pptr) ;
            
            if( !( pptr->GetPVAtk() ) )//mine(pv)
            {
                dd++ ;
                PathR[i].SetPVMine(true)     ;
            }
            else{   pptr->SetPVCand(true)   ;   }
        }else{
            pptr->safe_ctr = pptr->safe_ctr+1 ;
        }
    }
    
    if( flag )
    {
        printf("Instance(PV) : PI(C) = %d , PO(C) = %d , C = %d , M = %d \n" , aa ,bb,aa+bb+cc-dd,dd ) ;
    }
    return;
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

void PV_Monte_Simulation( int PVtimes , double year , vector< struct PVdata* > &_vPV, double bu, double bl  )
{
    FILE *fupper100 = fopen("./quality/Q_upper100.txt","w+t") ;
    FILE *flower100 = fopen("./quality/Q_lower100.txt","w+t") ;
    
    double PV_monteU = 0, PV_monteL = 0 ;
    double L = year-ERROR, R = year+ERROR ;

    //-----------------------------------------------------------------------------------
    for( int i = 0 ; i < PVtimes ; i++ )
    {
        PV_monteU = PV_monteL = 0.0              ;
        printf( YELLOW "[ %d th Instance Simulation ]\n" RESET , i )   ;
        Monte_PVCalQuality( year, PV_monteU, PV_monteL )      ;
        printf( "Q(PV) : ") ;
        if( PV_monteU < year - ERROR ) printf( RED )  ;
        else                           printf( RESET );
        printf("%f ~ ", PV_monteU );
        if( PV_monteL > year + ERROR ) printf( GRN )  ;
        else                           printf( RESET );
        printf("%f\n", PV_monteL );
        printf( RESET );
        Region( PV_monteU, PV_monteL, L, R ) ;
        fprintf( fupper100, "%f\n", PV_monteU )          ;
        fprintf( flower100, "%f\n", PV_monteL )          ;
    }
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
            PathR[i].gTiming(j)->setVth_pv( Z*0.01 )                ;//0.01 mean 10mv
        }
    }
}
////////////////////////////////////////////////////////////////////////////////////
//              PV Simulator - Quality Calculator                                 //
////////////////////////////////////////////////////////////////////////////////////
double Monte_PVCalQuality(double year, double &up, double &low)
{
    up = 10.0; low = 0.0            ;
    int TryT = 3000 / PathC.size()  ;
    if( TryT < 30 ) TryT = 30       ;
    map< double , worse* > worse    ;
    vector< double> monte           ;
    monte.clear( )                  ;
    
    //-------------------- Generate a New Instance with PV --------------------------
    GeneratePVCkt( )                ;
    
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

bool Condition( PATH &pptr )
{
    if( pptr.GetPreCtr() > 0 || pptr.GetPosCtr() > 0 || pptr.GetPVCandTMine() > 0 ||pptr.GetPVShTMine() > 0 || pptr.GetPVShTSafe() > 0 )
        return true     ;
    if( pptr.GetPVCandTSafe() > 0 ||  pptr.GetSafeTCand() > 0 || pptr.GetSafeTMine() > 0 || pptr.GetCandTCand() > 0  )
        return true     ;
    if( pptr.GetMineTCand() > 0 || pptr.GetMineTMine() > 0 || pptr.GetMineTSafe() > 0 )
        return true     ;
    else return false   ;
}

void printDCCType( FILE *fout , struct dccinfo * PDP , int D )
{
    if( D == 0 )//left
    {
        if( PDP->x == DCC_NONE ) fprintf( fout , "( None  ) " ) ;
        if( PDP->x == DCC_F    ) fprintf( fout , "( 80 %s ) ", p.c_str() ) ;
        if( PDP->x == DCC_S    ) fprintf( fout , "( 20 %s ) ", p.c_str() ) ;
        if( PDP->x == DCC_M    ) fprintf( fout , "( 40 %s ) ", p.c_str() ) ;
    }
    else//right
    {
        if( PDP->y == DCC_NONE ) fprintf( fout , "( None  ) " ) ;
        if( PDP->y == DCC_F    ) fprintf( fout , "( 60 %s ) ", p.c_str() ) ;
        if( PDP->y == DCC_S    ) fprintf( fout , "( 20 %s ) ", p.c_str() ) ;
        if( PDP->y == DCC_M    ) fprintf( fout , "( 40 %s ) ", p.c_str() ) ;
    }
}
void PrintRoleDisribute( FILE *fout, PATH &pptr )
{
    fprintf( fout, RED  "Mine      " CYAN " = " RED  "%d/%d ( %f %s )\n" , pptr.mine_ctr, pv_times, ((double)pptr.mine_ctr/(double)pv_times)*100 , p.c_str() ) ;
    fprintf( fout, BLUE "Candidate " CYAN " = " BLUE "%d/%d ( %f %s )\n" , pptr.cand_ctr, pv_times, ((double)pptr.cand_ctr/(double)pv_times)*100 , p.c_str()) ;
    fprintf( fout, GRN  "Safe      " CYAN " = " GRN  "%d/%d ( %f %s )\n" , pptr.safe_ctr, pv_times, ((double)pptr.safe_ctr/(double)pv_times)*100 , p.c_str()) ;
}
void PrintDCCOnPath( FILE *fout , PATH &pptr )
{
    GATE *stdptr = pptr.Gate( 0 )               ;
    GATE *endptr = pptr.Gate( pptr.length()-1 ) ;
    bool left = false  ;
    bool right= false  ;
    //----------------------------------------------------------------------------------
    for( int i = 0 ; i < stdptr->ClockLength() ; i++ )
    {
        auto itr = find( _vDCCGate.begin(), _vDCCGate.end(), stdptr->GetClockPath(i) ) ;
        if( itr != _vDCCGate.end() )
        {
            left = true ;
            fprintf( fout, CYAN "[ %s " , (*itr)->GetName().c_str() ) ;
            if( (*itr)->GetDcc() == DCC_M ){ fprintf( fout, CYAN "( 40 %s ) ", p.c_str() ) ; }
            if( (*itr)->GetDcc() == DCC_S ){ fprintf( fout, CYAN "( 20 %s ) ", p.c_str() ) ; }
            if( (*itr)->GetDcc() == DCC_F ){ fprintf( fout, CYAN "( 80 %s ) ", p.c_str() ) ; }
        }
    }
    if( !left ){ fprintf( fout, CYAN "[ None ( None ) ," ) ; }
    //----------------------------------------------------------------------------------
    for( int i = 0 ; i < endptr->ClockLength() ; i++ )
    {
        auto itl = find( _vDCCGate.begin(), _vDCCGate.end(), endptr->GetClockPath(i) ) ;
        if( itl != _vDCCGate.end() )
        {
            right = true ;
            fprintf( fout, CYAN " %s ", (*itl)->GetName().c_str() ) ;
            if( (*itl)->GetDcc() == DCC_M ){ fprintf( fout, CYAN "( 40 %s ) ]\n", p.c_str() ) ; }
            if( (*itl)->GetDcc() == DCC_S ){ fprintf( fout, CYAN "( 20 %s ) ]\n", p.c_str() ) ; }
            if( (*itl)->GetDcc() == DCC_F ){ fprintf( fout, CYAN "( 80 %s ) ]\n", p.c_str() ) ; }
        }
    }
    if( !right ){ fprintf( fout, CYAN " None ( None ) ]\n" ) ; }
}

////////////////////////////////////////////////////////////////////////////////////
//              PDP Analyzer                                                      //
////////////////////////////////////////////////////////////////////////////////////
void PDPF( FILE *fout , PATH & pptr )
{   //PDPF:PDP File
    int j = 0 ;
    vector<tuple<int,int>> left  ;//Left  DCC placement when right is None-Dcc.
    vector<tuple<int,int>> right ;//Right DCC placement when left  is None-Dcc.
    fprintf(fout, "\n" CYAN "**********************************************************************\n" ) ;
    fprintf(fout, CYAN "**********************************************************************\n" ) ;
    //-------------------------------------------------------------------------------------------------------------------
    if( pptr.GetCand()   ){ fprintf(fout,"OC ")  ; }
    if( pptr.Is_Chosen() ){ fprintf(fout,"OS ")  ; }
    if( pptr.IsSafe()    ){ fprintf(fout,"OF ")  ; }
    if( pptr.GetMine()   ){ fprintf(fout,"OM ")  ; }
    //-------------------------------------------------------------------------------------------------------------------
    double pref_rate= ((double)pptr.GetPreCtr()/(double)GPre_Diver_Dot)*100 ;
    double posf_rate= ((double)pptr.GetPosCtr()/(double)GPos_Diver_Dot)*100 ;
    fprintf( fout, "[ %s ~ %s ] \n" ,LName,RName  )  ;
    PrintDCCOnPath( fout, pptr )                     ;
    PrintRoleDisribute( fout, pptr )                 ;
    fprintf( fout, CYAN "PDP = %d/%lu \n" RESET , pptr.pldcc , pptr._mapdcc.size() ) ;
    fprintf( fout, CYAN "Dots(" RED " Prefail PathJ " CYAN ") : " RED "%d/%d ( %f %s ) \n" RESET ,pptr.GetPreCtr(),GPre_Diver_Dot,pref_rate,p.c_str() )  ;
    fprintf( fout, CYAN "Dots(" GRN " Posfail PathJ " CYAN ") : " GRN "%d/%d ( %f %s ) \n" RESET ,pptr.GetPosCtr(),GPos_Diver_Dot,posf_rate,p.c_str() )  ;
    //--------------------------------------------------------------------------------------------------------------------
    for( auto PDP : pptr._vPDP )
    {
        //------------------ Trivial Info (Skip) -------------------------------------------------------------------------
        if( PDP->prefail ) continue ;
        else if( PDP->pv_prefail == 0 ) continue ;
        //------------------ Repeated Info (Skip) ------------------------------------------------------------------------
        if( (int)PDP->x == 0 )//left is none-dcc
        {
            auto itR = find(right.begin(),right.end(),tuple<int,int>(PDP->idy,(int)PDP->y)) ;
            if( itR != right.end() ) continue ;
        }
        if( (int)PDP->y == 0 )//right is none-dcc
        {
            auto itL = find(left.begin(),left.end(),tuple<int,int>(PDP->idx,(int)PDP->x)) ;
            if( itL != left.end() ) continue ;
        }
        
        fprintf( fout, CYAN "----------------------------------------------------------------------\n" ) ;
        fprintf( fout, CYAN "(%d) [ %d, %d, %d, %d, %d ] [" RESET , j, pptr.GetPathID(), PDP->idx, PDP->idy, (int)PDP->x, (int)PDP->y );
        //----------------------------------------------------------------------------------------------------------------
        auto itL = find( _vDCCGate.begin(), _vDCCGate.end() , PDP->LGate ) ;
        auto itR = find( _vDCCGate.begin(), _vDCCGate.end() , PDP->RGate ) ;
        if( itL != _vDCCGate.end() && ((*itL)->GetDcc()) == PDP->x ) fprintf( fout, YELLOW  " %s " ,PDP->L_Name.c_str() ) ;
        else                                                         fprintf( fout, CYAN    " %s " ,PDP->L_Name.c_str() ) ;
        printDCCType( fout, PDP, 0 ) ;
        if( itR != _vDCCGate.end() && ((*itR)->GetDcc()) == PDP->y ) fprintf( fout, YELLOW  "%s "  ,PDP->R_Name.c_str() ) ;
        else                                                         fprintf( fout, CYAN    "%s "  ,PDP->R_Name.c_str() ) ;
        printDCCType( fout, PDP, 1 ) ;
        //-----------------------------------------------------------------------------------------------------------------
        if( PDP->middle  ){ fprintf( fout, CYAN "] $\n" RESET ) ; }
        else              { fprintf( fout, CYAN "] \n"  RESET ) ; }
        //-----------------------------------------------------------------------------------------------------------------
        if( PDP->middle  ){ fprintf( fout, CYAN "  Original : " BLUE "Middle  \n" RESET ) ;  }
        if( PDP->prefail ){ fprintf( fout, CYAN "  Original : " RED  "PreFail \n" RESET ) ;  }
        if( PDP->posfail ){ fprintf( fout, CYAN "  Original : " GRN  "PosFail \n" RESET ) ;  }
        //-----------------------------------------------------------------------------------------------------------------
        double prefrate = ((double)PDP->pv_prefail/(double)pv_times)*100 ;
        double middrate = ((double)PDP->pv_middle /(double)pv_times)*100 ;
        double posfrate = ((double)PDP->pv_posfail/(double)pv_times)*100 ;
        if( PDP->pv_prefail > 0 ){ fprintf(fout, CYAN "  Instance(" RED  "  PreFail " CYAN ") =" RED  " %d/%d ( %f %s )\n" RESET,PDP->pv_prefail,pv_times,prefrate,p.c_str() ); }
        if( PDP->pv_middle  > 0 ){ fprintf(fout, CYAN "  Instance(" BLUE "  Middle  " CYAN ") =" BLUE " %d/%d ( %f %s )\n" RESET,PDP->pv_middle ,pv_times,middrate,p.c_str() ); }
        if( PDP->pv_posfail > 0 ){ fprintf(fout, CYAN "  Instance(" GRN  "  PosFail " CYAN ") =" GRN  " %d/%d ( %f %s )\n" RESET,PDP->pv_posfail,pv_times,posfrate,p.c_str() ); }
        //-----------------------------------------------------------------------------------------------------------------
        if( (int)PDP->x == 0 ){ right.push_back( tuple<int,int>( PDP->idy,(int)PDP->y) ) ; }//Record it to avoid the next repeated DCC placement.
        if( (int)PDP->y == 0 ){ left.push_back(  tuple<int,int>( PDP->idx,(int)PDP->x) ) ; }
        j++ ;
    }
}
////////////////////////////////////////////////////////////////////////////////////
//              PV Path Summary                                                   //
////////////////////////////////////////////////////////////////////////////////////
void PV_show()
{
    int ttl_ctr   = 0 ,pre_ctr   = 0 ,pos_ctr   = 0 ;
    int safetcand = 0 ,safetmine = 0 ,candtcand = 0 ;
    int mtm       = 0 ,mtc       = 0 ,mtf       = 0 ;
    int trn_cand_ctr    = 0 ,trn_sh_ctr         = 0 ;
    int trn_shtsafe_ctr = 0 ,trn_candtsafe_ctr  = 0 ;
    int shrt  = shortlistsize()                     ;
    int cands = info[1]+info[2]+info[3]-info[4]     ;
    double PDP_thre = 5 ;
    //-----------------------------------------------------------------------------
    FILE *fout  = fopen( "./analysis/PDP.txt" ,"w+t")    ;
    FILE *fout2 = fopen( "./analysis/Path.txt","w+t")    ;
    fprintf( fout2, CYAN "\n------------PV Path Investigation----------------\n" ) ;
   //-----------------------------------------------------------------------------
    for( auto pptr : PathR )
    {
        bool PDP = false      ;
        bool t   = false      ;
        if( Condition( pptr ) )
        {
            fprintf( fout2, CYAN "-------------------------------------------------\n" ) ;
            if( pptr.Is_Chosen() )  fprintf( fout2, "OS ")   ;
            if( pptr.GetCand()   )  fprintf( fout2, "OC ")   ;
            if( pptr.GetMine()   )  fprintf( fout2, "OM ")   ;
            if( pptr.IsSafe()    )  fprintf( fout2, "OF ")   ;
            fprintf( fout2, "[ %s ~ %s ] ",LName, RName )    ;
            PrintDCCOnPath( fout2, pptr )                    ;
            PrintRoleDisribute( fout2, pptr )                ;

            fprintf( fout2, CYAN "-PDP = %d/%ld \n" RESET, pptr.pldcc , pptr._mapdcc.size() ) ;
            t = true  ;
            ttl_ctr++ ;
        }
        //-----------------------------------------------------------------------------
        if( pptr.GetPreCtr() > 0 )
        {
            double avg = (pptr.GetDijPVPre()/pptr.GetPreCtr())*100 - 100 ;
            double max = (pptr.GetDijPreMax())*100 - 100                 ;
            double min = (pptr.GetDijPreMin())*100 - 100                 ;
            double rate= (double)pptr.GetPreCtr()/(double)GPre_Diver_Dot ;
            rate *= 100 ;
            fprintf( fout2, CYAN "-Dots(" RED " Prefail PathJ " CYAN ") : " RED "%d/%d ( %f %s ) \n",pptr.GetPreCtr(),GPre_Diver_Dot,rate,p.c_str() )  ;
            fprintf( fout2, CYAN "-DijRate : " RED "%f %s(Avg), %f %s(Max), %f %s(Min) \n" RESET, avg,p.c_str(),max,p.c_str(),min,p.c_str() );
            if( rate >= PDP_thre ){ PDP = true ; }
            pre_ctr++ ;
        }
        if( pptr.GetPosCtr() > 0 )
        {
            double avg = (pptr.GetDijPVPos()/pptr.GetPosCtr())*100 - 100 ;
            double max = (pptr.GetDijPosMax())*100 - 100                 ;
            double min = (pptr.GetDijPosMin())*100 - 100                 ;
            double rate= (double)pptr.GetPosCtr()/(double)GPos_Diver_Dot ;
            rate *= 100 ;
            fprintf( fout2, CYAN "-Dots(" GRN " Posfail PathJ " CYAN ") : " GRN "%d/%d ( %f %s ) \n",pptr.GetPosCtr(),GPos_Diver_Dot,rate,p.c_str() )  ;
            fprintf( fout2, CYAN "-DijRate : " GRN "%f %s(Avg), %f %s(Max), %f %s(Min) \n" RESET,avg,p.c_str(),max,p.c_str(),min,p.c_str() ) ;
            if( rate >= PDP_thre ){ PDP = true ; }
            pos_ctr++ ;
        }
        //-----------------------------------------------------------------------------
        //C = Candidate, M = Mine, F = Safe
        fprintf( fout2, CYAN ) ;
        if( pptr.GetPVCandTMine() > 0 ){
            fprintf( fout2, "-Diverge because C-->M = %d \n", pptr.GetPVCandTMine() )      ;
            trn_cand_ctr++ ;
        }
        if( pptr.GetPVShTMine() > 0 ){
            fprintf( fout2, "-Diverge because S-->M = %d \n", pptr.GetPVShTMine() )        ;
            trn_sh_ctr++ ;
        }
        if( pptr.GetPVShTSafe() > 0 ){
            fprintf( fout2, "-Diverge because S-->F = %d \n", pptr.GetPVShTSafe() )        ;
            trn_shtsafe_ctr++ ;
        }
        if( pptr.GetPVCandTSafe() > 0 ){
            fprintf( fout2, "-Diverge because C-->F = %d \n", pptr.GetPVCandTSafe() )      ;
            trn_candtsafe_ctr++ ;
        }
        if( pptr.GetSafeTCand() > 0 ){
            fprintf( fout2, "-Diverge because F-->C = %d \n", pptr.GetSafeTCand() )        ;
            safetcand++ ;
        }
        if( pptr.GetSafeTMine() > 0 ){
            fprintf( fout2, "-Diverge because F-->M = %d \n", pptr.GetSafeTMine() )        ;
            safetmine++ ;
        }
        if( pptr.GetCandTCand() > 0 ){
            fprintf( fout2, "-Diverge because C-->C = %d \n", pptr.GetCandTCand() )        ;
            candtcand++ ;
        }
        if( pptr.GetMineTMine() > 0 ){
            fprintf( fout2, "-Diverge because M-->M = %d \n", pptr.GetMineTMine() )        ;
            mtm++ ;
        }
        if( pptr.GetMineTCand() > 0 ){
            fprintf( fout2, "-Diverge because M-->C = %d \n", pptr.GetMineTCand() )        ;
            mtc++ ;
        }
        if( pptr.GetMineTSafe() > 0 ){
            fprintf( fout2, "-Diverge because M-->F = %d \n", pptr.GetMineTSafe() )        ;
            mtf++ ;
        }
        //-----------------------------------------------------------------------------
        fprintf( fout2, RESET ) ;
        if( t )     fprintf( fout2, "\n" )   ;
        if( PDP )   PDPF( fout, pptr )       ;
    }
    //-----------------------------------------------------------------------------
    fprintf( fout2, CYAN ) ;
    fprintf( fout2, "-----Paths which has led to divergence-------- \n"   ) ;
    fprintf( fout2, "TTL Paths : %d \n" , ttl_ctr  )         ;
    fprintf( fout2, "Prefail Paths # : %d \n" , pre_ctr )    ;//曾經提早死掉的path累計總數
    fprintf( fout2, "Posfail Paths # : %d \n" , pos_ctr )    ;//曾經延後死掉的path累計總數
    fprintf( fout2, "C -> M(PV) Paths # : %d/%d  \n" , trn_cand_ctr, cands )       ;//轉變後，造成年份發散的Path累計數量
    fprintf( fout2, "C -> F(PV) Paths # : %d/%d  \n" , trn_candtsafe_ctr , cands ) ;
    fprintf( fout2, "C -> C(PV) Paths # : %d/%d  \n" , candtcand , cands )         ;
    fprintf( fout2, "S -> M(PV) Paths # : %d/%d  \n" , trn_sh_ctr , shrt )         ;
    fprintf( fout2, "S -> F(PV) Paths # : %d/%d  \n" , trn_shtsafe_ctr , shrt )    ;
    fprintf( fout2, "F -> C(PV) Paths # : %d\n" , safetcand ) ;
    fprintf( fout2, "F -> M(PV) Paths # : %d\n" , safetmine ) ;
    fprintf( fout2, "M -> C(PV) Paths # : %d\n" , mtc )       ;
    fprintf( fout2, "M -> M(PV) Paths # : %d\n" , mtm )       ;
    fprintf( fout2, "M -> F(PV) Paths # : %d\n" , mtf )       ;
    fprintf( fout2, RESET ) ;
}

