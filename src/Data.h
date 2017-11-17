
#ifndef Data_hpp
#define Data_hpp

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <math.h>
#include <assert.h>
#include "circuit.h"
using namespace std ;

class PVdata    ;
class HASHTABLE ;
class GATE      ;
class PATH      ;
class CIRCUIT   ;

inline double maxf(double a, double b) ;
inline double minf(double a, double b) ;
bool    BInv(double&,double&,double,double,double,double,double,int&,int,int )      ;
void    ReadParameter( int ,char*[])    ;
void    printSetting( )                 ;
void    release( HASHTABLE *)                                                       ;
void    PV_show()                                                                   ;
void    PV_Monte_Simulation( double, double )          ;
void    ReverseSol(  )    ;
void    RemoveAdditionalDCC( bool * ) ;
void    AddNode( ) ;

//////////////////////////////////////////////////////////////////
//          Varaible Declaration                                //
//////////////////////////////////////////////////////////////////
extern double **EdgeA          ;
extern double **EdgeB          ;
extern double **cor            ;
extern double **ser            ;
extern double info[5]          ;
extern double ERROR            ;
extern double PVRange          ;
extern double PLUS             ;
extern double tight            ;

//////////////////////////////////////////////////////////////////
extern vector<PATH>  PathR     ;
extern vector<PATH*> PathC     ;
extern vector<PATH*> _vPathC   ;
extern vector<CIRCUIT> Circuit ;
//////////////////////////////////////////////////////////////////
extern int    TotalTimes       ;
extern int    Threshold        ;
extern string fname            ;
//////////////////////////////////////////////////////////////////

//--------------- Quality for Each Instance ----------------------
struct PVdata
{
    double  upper,lower,dist    ;
    //------------ Constructor -----------------------------------
    PVdata( double u , double l ):upper(u),lower(l)
    {  dist = 0 ; }
    PVdata( ){  upper = lower = dist = 0        ; }
    //------------ Function --------------------------------------
    double      gupper()          { return upper; }
    double      glower()          { return lower; }
    double      gdist()           { return dist ; }
    void        supper( double u ){ upper = u   ; }
    void        slower( double l ){ lower = l   ; }
    void        sdist( double d ) { dist  = d   ; }
};

//--------------- Info for Each Dots ------------------------------
struct worse
{
    worse( PATH *pi ,PATH *pj, double d ):_pPi(pi),_pPj(pj),_DijDelta(d)
    {}
    PATH* _pPi ;
    PATH* _pPj ;
    double _DijDelta ;
};

//--------------- Info for DP --------------------------------------
struct dccinfo
{
    dccinfo( int ix , int iy, AGINGTYPE ax,AGINGTYPE ay ):idx(ix),idy(iy),x(ax),y(ay)
    {
        middle = prefail = false    ;
        posfail = true              ;
        pv_prefail = pv_posfail = pv_middle = 0 ;
        R_Name = L_Name = ""        ;
    }
    //---------------------------------------------------------------
    int  pv_prefail, pv_middle, pv_posfail  ;//ctr
    bool prefail   , middle   , posfail     ;
    int  idx, idy    ;
    AGINGTYPE x, y   ;
    GATE  *LGate, *RGate    ;
    string L_Name, R_Name   ;
    //-------------------------------------------------------------------------------------------
    void pv_pref( bool pv_pref ){ if( !(this->prefail) && ( pv_pref ) ){ this->pv_prefail++ ; } }
    void pv_posf( bool pv_posf ){ if( !(this->posfail) && ( pv_posf ) ){ this->pv_posfail++ ; } }
    void pv_midd( bool pv_midd ){ if( !(this->middle ) && ( pv_midd ) ){ this->pv_middle++  ; } }
};

struct info
{
    int bestdcc ;
    int dccs ;
    int oridccs  ;
    double bestup ;
    double bestlow;
    double upper, lower ;
    info()
    {
        bestup   = 100   ;
        bestlow  = -100  ;
        bestdcc  = 10000 ;
        dccs     = 0     ;
        oridccs  = 0    ;
    }
};
















#endif /* Data_hpp */
