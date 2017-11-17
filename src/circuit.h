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
class GATE      ;
class PATH      ;
//struct dccinfo  ;
/////////////////////////////////////////////////////////////////////////
//                  Function Forward Declration                        //
/////////////////////////////////////////////////////////////////////////
void ReadCircuit( string )          ;
void ReadPath_l( string )           ;
void CalVertexWeight( )             ;
void RemoveRDCCs( )                 ;
void EstimateTimeEV(double )        ;
void ReadCpInfo( string )           ;
void PrintStatus( double )          ;
void AdjustConnect( )               ;
void CheckOriLifeTime( )            ;
void GenerateSAT( string ,double  ) ;
void printDCCLocation() ;
void CheckPathAttackbility( )      ;
void DefCandMineSafe( double, double, bool, double )            ;
bool Vio_Check( PATH*,int,int,AGT,AGT,double,int,int,int )      ;
bool Vio_Check( PATH*,long double,double,int,int,int)           ;
/////////////////////////////////////////////////////////////////////////
int RefineResult( double ,bool  )   ;
int HashAllClockBuffer( )           ;
int CallSatAndReadReport( int )     ;
/////////////////////////////////////////////////////////////////////////
bool CheckNoVio( double )           ;
bool AnotherSol( )                  ;
bool ChooseVertexWithGreedyMDS( double ,bool  , HASHTABLE *  )  ;
/////////////////////////////////////////////////////////////////////////
double Monte_PVCalQuality( double &, double &)          ;
double Monte_CalQuality(double , double &, double &, int )      ;
double CalPreAgingwithPV( double, int, int, double )            ;
double CalPreAging( double, int, int, double )                  ;
double CalQuality( double &, double &, int )                    ;
double absff( double )                                          ;
void   ReadVth_pv_Sv( )                                         ;
double FindSv( double Vth_pv )                                  ;
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
    unsigned CalKey() ;
    bool Exist() ;
    void PutNowStatus();
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
    map< string, GATE* >nametogate  ;
    map< string, WIRE* >nametowire  ;
public:
    CIRCUIT(string n):name(n)
    {
        gate_list.clear();
        wire_list.clear();
    }
    string GetName()
    {
        return name;
    }
    void PutWire( WIRE* w)
    {
        wire_list.push_back(w);
        nametowire[w->GetName()] = w;
    }
    void PutGate( GATE* g )
    {
        gate_list.push_back(g);
        nametogate[g->GetName()] = g;
    }
    WIRE* GetWire(int i)
    {
        return wire_list[i];
    }
    WIRE* GetWire(string name)
    {
        if( nametowire.find(name) == nametowire.end() )
        {
            WIRE* t = new WIRE(name, PI);
            wire_list.push_back(t)  ;
            nametowire[name] = t    ;
            cout << name << endl    ;
        }
        return nametowire[name];
    }
    GATE* GetGate( string name )
    {
        if (nametogate.find(name) == nametogate.end())	return NULL;
        return nametogate[name];
    }
    GATE* GetGate( int i )
    {
        return gate_list[i];
    }
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
    bool            pv_attackable   ;
    bool            safe            ;
    bool            pv_safe         ;
    bool            choose          ;
    bool            tried           ;//Used for refine
    bool            mine            ;//
    bool            pv_mine         ;
    bool            cand            ;
    bool            pv_cand         ;
    unsigned        no              ;
    PATHTYPE        type            ;
    int             pv_pre_ctr      ;
    int             pv_pos_ctr      ;
    int             pv_candtmine_ctr;
    int             pv_shtmine_ctr  ;
    int             pv_shtsafe_ctr  ;
    int             pv_candtsafe_ctr;
    int             pv_safetcand_ctr;
    int             pv_safetmine_ctr;
    int             pv_candtcand_ctr;
    int             pv_mtm          ;//mine to mine
    int             pv_mtc          ;//mine to candidate
    int             pv_mtf          ;//mine to safe
    int             path_id         ;
    double          dij_delta_pre   ;
    double          dij_delta_pre_mn;
    double          dij_delta_pre_mx;
    double          dij_delta_pos   ;
    double          dij_delta_pos_mx;
    double          dij_delta_pos_mn;
    
public:
    //--------------------Constructor/Destructor-------------------------//
    int pldcc                           ;
    int cand_ctr                        ;
    int mine_ctr                        ;
    int safe_ctr                        ;
    vector< struct dccinfo * > _vPDP    ;
    map< tuple<int,int,AGT,AGT>,struct dccinfo * > _mapdcc ;
    PATH():attackable(false),choose(false)
    {
        mine = false                    ;
        setuptime = holdtime = -1       ;
        gate_list.clear()               ;
        timing.clear()                  ;
        pv_pre_ctr = pv_pos_ctr = 0     ;
        mine = cand = pv_mine = false   ;
        pv_candtmine_ctr = 0            ;
        pv_shtmine_ctr = 0              ;
        pv_shtsafe_ctr = 0              ;
        pv_candtsafe_ctr = 0            ;
        pv_attackable = false           ;
        pv_safetcand_ctr = 0            ;
        pv_safetmine_ctr = 0            ;
        pv_candtcand_ctr = 0            ;
        pv_mtm = pv_mtc = pv_mtf = 0    ;
        dij_delta_pre = 0               ;
        dij_delta_pos = 0               ;
        dij_delta_pre_mn = 100          ;
        dij_delta_pre_mx = -100         ;
        dij_delta_pos_mn = 100          ;
        dij_delta_pos_mx = -100         ;
        pldcc = 0                       ;
        cand_ctr = mine_ctr = safe_ctr=0;
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
    double      GetDijPreMin( )     { return dij_delta_pre_mn    ;  }
    double      GetDijPosMin( )     { return dij_delta_pos_mn    ;  }
    void        SetDijPreMin( double t){ if( t < dij_delta_pre_mn ) dij_delta_pre_mn = t ; }
    void        SetDijPosMin( double t){ if( t < dij_delta_pos_mn ) dij_delta_pos_mn = t ; }
    double      GetDijPreMax( )     { return dij_delta_pre_mx    ;  }
    double      GetDijPosMax( )     { return dij_delta_pos_mx    ;  }
    void        SetDijPreMax( double t){ if( t > dij_delta_pre_mx ) dij_delta_pre_mx = t ; }
    void        SetDijPosMax( double t){ if( t > dij_delta_pos_mx ) dij_delta_pos_mx = t ; }
    double      In_time( int i )    { return timing[i].in_time() ;  }
    double      Out_time( int i )   { return timing[i].out_time();  }
    double      PV_time( int i )    { return timing[i].pv()      ;  }
    void        SetNo( unsigned n ) { no =  n       ; }	//記錄在timing report上的path號碼
    void        SetDijPVPre( double i ){ dij_delta_pre += i ; }
    double      GetDijPVPre( )      { return dij_delta_pre  ; }
    void        SetDijPVPos( double i ){ dij_delta_pos += i ; }
    double      GetDijPVPos( )      { return dij_delta_pos  ; }
    unsigned    No( )               { return no     ; }
    void        SetST( double t )   { setuptime = t ; }	//setup time
    void        SetHT( double t )   { holdtime = t  ; }
    void        SetCandTCand()      { pv_candtcand_ctr++;  }
    int         GetCandTCand()      { return pv_candtcand_ctr ; }
    void        SetSafeTCand()      { pv_safetcand_ctr++ ; }
    int         GetSafeTCand()      { return pv_safetcand_ctr ; }
    void        SetSafeTMine()      { pv_safetmine_ctr++ ; }
    int         GetSafeTMine()      { return pv_safetmine_ctr ; }
    void        SetMineTCand()      { pv_mtc++      ; }
    int         GetMineTCand()      { return pv_mtc ; }
    void        SetMineTSafe()      { pv_mtf++      ; }
    int         GetMineTSafe()      { return pv_mtf ; }
    void        SetMineTMine()      { pv_mtm++      ; }
    int         GetMineTMine()      { return pv_mtm ; }
    void        SetPreCtr( )        { pv_pre_ctr++  ; }
    void        SetPosCtr( )        { pv_pos_ctr++  ; }
    void        SetMine( bool t )   { mine = t      ; }
    void        SetPVMine( bool t ) { pv_mine = t   ; if( t )    mine_ctr++ ; }
    void        SetCand( bool t )   { cand = t      ; }
    void        SetPVCand( bool t ) { pv_cand = t   ; if( t ) cand_ctr++ ; }
    void        SetPVSafe( bool t ) { pv_safe = t   ; }
    void        SetPVAtk( bool t )  { pv_attackable = t ; }
    bool        GetCand( )          { return cand   ; }
    bool        GetMine( )          { return mine   ; }
    bool        GetPVMine( )        { return pv_mine; }
    bool        GetPVSafe( )        { return pv_safe; }
    bool        GetPVCand( )        { return pv_cand; }
    bool        GetPVAtk()         { return pv_attackable ; }
    void        SetPVCandTMine( )   { pv_candtmine_ctr++; }
    void        SetPVCandTSafe( )   { pv_candtsafe_ctr++; }
    void        SetPVShTMine( )     { pv_shtmine_ctr++  ; }
    void        SetPVShTSafe( )     { pv_shtsafe_ctr++  ; }
    int         GetPVCandTMine( )   { return pv_candtmine_ctr ; }
    int         GetPVCandTSafe( )   { return pv_candtsafe_ctr ; }
    int         GetPVShTMine( )     { return pv_shtmine_ctr   ; }
    int         GetPVShTSafe( )     { return pv_shtsafe_ctr   ; }
    int         GetPreCtr( )        { return pv_pre_ctr ; }
    int         GetPosCtr( )        { return pv_pos_ctr ; }
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
