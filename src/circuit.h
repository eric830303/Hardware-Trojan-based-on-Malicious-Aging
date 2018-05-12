#ifndef CIRCUIT_H
#define CIRCUIT_H
#include<iostream>
#include<fstream>
#include<sstream>
#include<math.h>
#include<cmath>
#include<stdlib.h>
#include<algorithm>
#include<time.h>
#include<string>
#include<vector>
#include<map>
#include<tuple>
#include"typedef.h"
#include"aging.h"
#include"Data.h"
#include<pthread.h>

#define AGT AGINGTYPE
using namespace std ;

/////////////////////////////////////////////////////////////////////////
//                  Class Forward Declration                           //
/////////////////////////////////////////////////////////////////////////
class HASHTABLE ;
class CIRCUIT   ;
class GATE      ;
class PATH      ;
//struct dccinfo  ;
/////////////////////////////////////////////////////////////////////////
//                  Function Forward Declration                        //
/////////////////////////////////////////////////////////////////////////

void CalVertexWeight( )             ;
void RemoveRDCCs( )                 ;
void EstimateTimeEV(double )        ;
void ReadCpInfo( string )           ;
void PrintStatus( double )          ;

void DefCandMineSafe( double, double, bool, double )            ;
double absff( double )                                          ;
void   CalSv( )                                         ;
double CalAgingRateWithVthPV( double, double )                  ;
double CalPathAginRateWithPV( PATH *, double )                  ;
/////////////////////////////////////////////////////////////////////////
//                  HashTable                                          //
/////////////////////////////////////////////////////////////////////////
class HASHTABLE
{
private:
    bool*       exist   ;
    bool**      choose  ;
    unsigned    size    ;
public:
    HASHTABLE( unsigned s1 /*16*/, unsigned s2 /*PathC.size()*/)
    {   //s1是位元數
        size = s1   ;
        exist = new bool[ 1 << s1 ]     ;//1往左移動16bit
        choose = new bool *[ 1 << s1]   ;
        for (int i = 0; i < (1<<s1); i++)
        {
            exist[i] = false;
            choose[i] = new bool[s2];
        }
    }
    void clear()
    {
        for (int i = 0; i < (1<<size); i++)
        {
            delete choose[i] ;
        }
        delete choose ;
        delete exist  ;
        size = 0 ;
    }
    unsigned CalKey( CIRCUIT *) ;
    bool Exist( CIRCUIT * ) ;
    void PutNowStatus(  CIRCUIT * );
};

/////////////////////////////////////////////////////////////////////////
//                  Wire                                               //
/////////////////////////////////////////////////////////////////////////
class WIRE
{
private:
    WIRETYPE        type    ;
    string          name    ;
    GATE*           input   ;
    vector<GATE*>   output  ;
public:
    WIRE( string n,WIRETYPE t ) :name(n), type(t),input(NULL){ output.clear(); }
    ~WIRE(){ }
    WIRETYPE    GetType( )          { return type               ; }
    string      GetName( )          { return name               ; }
    GATE*       GetInput( )         { return input              ; }
    GATE*       GetOutput( int i )  { return output[i]          ; }
    int         No_Output( )        { return (int)output.size() ; }
    void        SetInput( GATE* g ) { input = g                 ; }
    void        SetOutput( GATE* g ){ output.push_back(g)       ; }
};

/////////////////////////////////////////////////////////////////////////
//                  Gate                                               //
/////////////////////////////////////////////////////////////////////////
class GATE
{
public:
    string  name                ;
    string  type                ;
    WIRE*   output              ;
    bool clock_flag             ;//Check whether FF's clk-path is complete or not.
    AGINGTYPE add_dcc           ;
    vector< WIRE* > input       ;
    vector< GATE* > clock_path  ;//It's used when the gate belong FF.
    double in_time, out_time    ;//Used only by clock-buffer.
    double in_out               ;
    double PV                   ;
    
    GATE(string n,string t) :name(n),type(t),in_time(-1),out_time(-1),clock_flag(false),add_dcc(DCC_NONE)
    {
        input.clear();
        output = NULL;
        clock_path.clear();
    }
    ~GATE(){}
    string      GetName( )              { return name       ;   }
    string      GetType( )              { return type       ;   }
    void        SetInput( WIRE* w )     { input.push_back(w);	}
    void        SetOutput( WIRE* w )    { output = w        ;   }
    WIRE*       GetInput( int i )       { return input[i]   ;   }
    WIRE*       GetOutput( )            { return output     ;   }
    void        SetInTime( double t )   { in_time = t       ;   }//Used fot intime,outtime(clock buffer timing)
    void        SetOutTime( double t )  { out_time = t      ;   }
    double      GetInTime( )            { return in_time    ;   }
    double      GetOutTime( )           { return out_time   ;   }
    void        Setflag( )              { clock_flag = true ;   }//Is it clock buffer ?
    void        SetDcc( AGINGTYPE dcc ) { add_dcc = dcc     ;   }//The DCC type on the clock gate.
    GATE*       GetClockPath( int i )   { return clock_path[i]; }
    int         No_Input( )             { return (int)input.size( )    ; }
    int         ClockLength( )          { return (int)clock_path.size(); }
    AGINGTYPE   GetDcc( )               { return add_dcc;       }
    void        SetClockPath( GATE* g )
    {
        if( clock_flag )
            return ;
        clock_path.push_back(g);
    }
};

/////////////////////////////////////////////////////////////////////////
//                  Circuit                                            //
/////////////////////////////////////////////////////////////////////////
class CIRCUIT
{
private:
    string name                     ;
    vector< GATE* >     gate_list   ;
    vector< WIRE* >     wire_list   ;
    vector< pair<double, double> > inst_LT;
    map< string, GATE* >nametogate  ;
    map< string, WIRE* >nametowire  ;
    HASHTABLE * _pHashTable         ;
    
    vector< GATE* > _vDCCGate ;//Store used DCC
    
    //-------- Results ------------------------------------------------
    int bestdcc ;
    int dccs ;
    int oridccs  ;
    double bestup ;
    double bestlow;
    double upper, lower ;
    //-------- Graph ------------------------------------------------
    double **EdgeA          ;
    double **EdgeB          ;
    double **cor            ;
    double **ser            ;
    double arc_thd = 0      ;
    //-------- Aging rate --------------------------------------------
    double Rate[15][4]      ;
    double A, alpha, Exp    ;
    //-------- PV ---------------------------------------------------
    double info[5]          ;
    double ERROR = 1.0      ;
    double PVRange = 0.00   ;
    
    double convergent_year  ;
    double VTH_CONVGNT[4]   ;//Each duty cycle correpond to one convergent Vth
    double Sv[4]            ;//Each duty cycle correpond to one Sv
    
    //-------- Timing ------------------------------------------------
    double year = 0         ;
    double period = 0       ;
    double tc_mgn = 0       ;
    double PLUS = 0.0       ;
    double tight = 1.000001 ;
    //-------- Path ------------------------------------------------
    vector<PATH>  _vPathAll  ;
    vector<PATH*> _vPathCand ;
    //-------- Setting ---------------------------------------------
    int    Q_mode           ;
    int    TotalTimes       ;
    int    Threshold        ;
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
    
    
    
public:
    CIRCUIT()
    {
        bestup   = 100   ;
        bestlow  = -100  ;
        bestdcc  = 10000 ;
        dccs     = 0     ;
        oridccs  = 0    ;
        convergent_year = year = ERROR = 0 ;
        gate_list.clear();
        wire_list.clear();
    }
    //---- Set/Put/Read ---------------------------------------------
    void PutWire( WIRE* ) ;
    void PutGate( GATE* );
    
    void ReadTimingReport( )  ;//Rename
    void ReadCircuit()   ;
    bool ReadParameter( int , char*[], string & );
    void ReadAgingData() ;
    void ReadCpInfo();
    
    void setHashTable( HASHTABLE * p ){ this->_pHashTable = p ; }
    void setBestUB( double U ){ bestup = U ; }
    void setBestLB( double L ){ bestup = L ; }
    void setBestDCCCount( int c ){ bestdcc = c ; }
    void setOriginDCCCount( int c ){ oridccs  = c ; }
    void setDCCCount(int c){ dccs = c ; }
    void setTryLimit(int t){ trylimit = t ; }
    void setName( string n ){ name = n ; }
    
    //---- Other Method (PV-related) --------------------------------
    void    PV_Monte_Simulation(  );
    void    GeneratePVCkt();
    double  Monte_PVCalQuality( double &up, double &low );
    
    //---- Other Method (Graph) ------------------------------------
    bool    MDS( bool puthash );
    bool    Check_Connect(int, int,double);
    bool    CheckNoVio( double year /* = (year+PLUS) in main.cpp */ );//Called in main.cpp for early checking.
    double  Overlap( int );
    void    AdjustConnect() ;
    
    //---- Other Method (Timing) ------------------------------------
    void    CheckOriLifeTime();
    bool    Vio_Check( PATH* pptr, int stn, int edn, AGT ast, AGT aed, double year );
    bool    Vio_Check( PATH* pptr, long double year, double Aging_P, bool  );
    double  CalQuality( double &up, double &low , int mode ) ;
    void    CalPreInv( double, double &,double &,int,int, double);
    double  CalPreAging( double, int, int, double);
    double  calConvergentVth( double, double );
    double  calSv( double, double, double );
    double  CalPathAginRateWithPV( PATH * pptr, double year );
    double  CalAgingRateWithVthPV( double, double );
    double  AgingRate(AGINGTYPE status, double year);
    //---- Other --------------------------------
    void    PathClassify();
    void    ReverseSol( );
    int     HashAllClockBuffer();
    void    RemoveRDCCs();
    void    release( HASHTABLE *hptr );
    void    printSetting(  );
    void    printDCCLocation();
    void    GenerateSAT( );
    int     CallSatAndReadReport( int );
    bool    BInv(double &bu, double &bl, double u1, double l1, double u2, double l2,double n,int &dcb,int dc1,int dc2);
    void    InstanceProab( );
    //----Get/Call --------------------------------------------------
    string  GetName() ;
    int     shortlistsize();
    double  getBestUB(  ){ return bestup ; }
    double  getBestLB(  ){ return bestlow  ; }
    double & getRBestUB(  ){ return bestup ; }
    double & getRBestLB(  ){ return bestlow  ; }
    double  getYear( )   { return year ; }
    int     getBestDCCCount(  ){ return bestdcc ; }
    int     getOriginDCCCount( ){ return oridccs   ; }
    int     getDCCCount(){ return dccs ; }
    int     getTryLimit(){ return trylimit ; }
    WIRE*   GetWire(int) ;
    WIRE*   GetWire(string);
    GATE*   GetGate(string);
    GATE*   GetGate( int i );
    vector<PATH>  & getPathALL() { return _vPathAll  ; }
    vector<PATH*> & getPathCand(){ return _vPathCand ; }
    vector<GATE*> & getDCCList() { return _vDCCGate  ; }//Store used DCC
    vector< pair<double,double> > & getInstLT() { return inst_LT  ; }//Store used DCC
    HASHTABLE * getHashTable( ){ return this->_pHashTable ; }
    void PutClockSource();
    
};

/////////////////////////////////////////////////////////////////////////
//                  Path                                               //
/////////////////////////////////////////////////////////////////////////
class PATH
{		//先不記trans time
private:
    //--------------------Timing ( Nested Class )----------------------//
    class TIMING
    {
    private:
        double in, out, Vth_pv ;
    public:
        TIMING(double i, double o , double p ) :in(i), out(o), Vth_pv(p)
        {  }
        ~TIMING(){}
        double in_time()        { return in ; }
        double out_time()       { return out; }
        double pv()             { return Vth_pv ; }
        void   setVth_pv( double p )  { Vth_pv = p    ; }
    } ;
    //--------------------Attribute------------------------------------//
    vector< GATE* > gate_list       ;
    vector< TIMING >timing          ;
    double          setuptime       ;
    double          holdtime        ;
    double          clock_to_end    ;
    double          estime          ;
    double          psd             ;//類標準差
    double          supper          ;
    double          slower          ;
    bool            attackable      ;
    bool            safe            ;
    bool            choose          ;
    bool            tried           ;//Used for refine
    bool            mine            ;//
    bool            cand            ;
    unsigned        no              ;
    PATHTYPE        type            ;
    int             path_id         ;
    
public:
    //--------------------Constructor/Destructor-------------------------//
    int pldcc                           ;
    vector< struct dccinfo * > _vPDP    ;
    map< tuple<int,int,AGT,AGT>,struct dccinfo * > _mapdcc ;
    PATH():attackable(false),choose(false)
    {
        mine = false                    ;
        setuptime = holdtime = -1       ;
        gate_list.clear()               ;
        timing.clear()                  ;
        pldcc = 0                       ;
        _vPDP.clear()                   ;
        _mapdcc.clear()                 ;
    }
    ~PATH(){}
    
    //--------------------Function---------------------------------------//
    void AddGate(GATE* g, double i ,double o, double p )
    {
        gate_list.push_back(g);
        TIMING temp( i, o, p );//in_time,out_time,PV
        timing.push_back(temp);
    }
    vector< TIMING > * gTiming()    { return &timing             ;  }
    TIMING *    gTiming( int i )    { return &timing[i]          ;  }
    GATE*       Gate( int i )       { return gate_list[i]        ;  }
    double      In_time( int i )    { return timing[i].in_time() ;  }
    double      Out_time( int i )   { return timing[i].out_time();  }
    double      PV_time( int i )    { return timing[i].pv()      ;  }
    void        SetNo( unsigned n ) { no =  n       ; }	//記錄在timing report上的path號碼
    unsigned    No( )               { return no     ; }
    void        SetST( double t )   { setuptime = t ; }	//setup time
    void        SetHT( double t )   { holdtime = t  ; }
    void        SetMine( bool t )   { mine = t      ; }
    void        SetCand( bool t )   { cand = t      ; }
    bool        GetCand( )          { return cand   ; }
    bool        GetMine( )          { return mine   ; }
    void        SetPathID( int i )  { path_id = i ; }
    int         GetPathID( )        { return path_id ; }
    void        SetCTE( double t )  { clock_to_end = t  ; }	//clock 到末端flip-flop的時間
    double      GetST( )            { return setuptime  ; }
    double      GetHT( )            { return holdtime   ; }
    double      GetCTE( )           { return clock_to_end   ; }
    double      GetCTH( )           { return timing[0].in_time()    ; }//clock到首端flip-flop的時間
    double      GetAT( )            { return timing[timing.size()-1].in_time() ; }	//arrival time
    double      GetFreshDij()       { return this->In_time( this->length() - 1 ) - this->Out_time(0) ;}
    void        SetType( PATHTYPE t ){ type = t ; }	//short or long path
    PATHTYPE    GetType( )          { return type; }
    int         length( )           { return (int)gate_list.size(); }//path的長度 不含clock
    void        SetAttack(bool a)   { attackable = a    ; }
    bool        CheckAttack( )      { return attackable ; }
    bool        Is_Chosen( )        { return choose     ; }
    void        SetChoose( bool c ) { choose = c        ; }
    void        SetEstimateTime( double t ){ estime = t ; }
    double      GetEstimateTime( )  { return estime     ; }
    void        SetPSD( double t )  { psd = t       ; }
    double      GetPSD( )           { return psd    ; }
    void        SetSafe( bool s )   { safe = s      ; }
    bool        IsSafe( )           { return safe   ; }
    void        SetTried( bool t )  { tried = t     ; }
    bool        IsTried( )          { return tried  ; }
};


#endif
