
#include "circuit.h"

#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define GRN     "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define RESET   "\x1b[0m"
#define thdnum  1

//////////////////////////////////////////////////////////////////
//          Varaible Declaration                                //
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////
//          Function Declaration                                //
//////////////////////////////////////////////////////////////////
double CIRCUIT::AgingRate(AGINGTYPE status, double year)
{
    
    double second = year * 365 * 86400;    //0.0039*(0.5*t)^0.2 + 1    average case -> worst caseßY•h±º0.5(alpha)
    int y = year;
    if( year > 10 )//DCC使用內插法計算老化率
    {
        switch (status)
        {
            case DCC_M:
                return (Rate[10][1] - Rate[9][1]) *(year - 10) + Rate[10][1];
            case DCC_NONE:
                return (Rate[10][3] - Rate[9][3]) *(year - 10) + Rate[10][3];
            case DCC_F:
                return (Rate[10][2] - Rate[9][2]) *(year - 10) + Rate[10][2];
            case DCC_S:
                return (Rate[10][0] - Rate[9][0]) *(year - 10) + Rate[10][0];
            case FF:
                return 0.02*year ;//F-F是假設每年老化率增加2%
            case WORST:
                return (this->A)*pow( (this->alpha)*second, this->Exp );
            case NORMAL:
                return (this->A)*pow( (this->alpha)*second, this->Exp );
            case BEST:
                return (this->A)*pow( 0.25*second, this->Exp );
            default:
                return (this->A)*pow( (this->alpha)*second, this->Exp );
        }
    }
    switch (status)
    {
        case DCC_M:
            if (year - (double)y > 0.00001)
                return (Rate[y + 1][1] - Rate[y][1]) *(year - (double)y) + Rate[y][1];
            return Rate[y][1];
        case DCC_NONE:
            if (year - (double)y > 0.00001)
                return (Rate[y + 1][3] - Rate[y][3]) *(year - (double)y) + Rate[y][3];
            return Rate[y][3];
        case DCC_F:
            if (year - (double)y > 0.00001)
                return (Rate[y + 1][2] - Rate[y][2]) *(year - (double)y) + Rate[y][2];
            return Rate[y][2];
        case DCC_S:
            if (year - (double)y > 0.00001)
                return (Rate[y + 1][0] - Rate[y][0]) *(year - (double)y) + Rate[y][0];
            return Rate[y][0];
        case FF:
            return 0.02*year;
        case WORST:
            return (this->A)*pow( (this->alpha)*second, this->Exp );
        case NORMAL:
            return (this->A)*pow( (this->alpha)*second, this->Exp );
        case BEST:
            return (this->A)*pow( 0.25*second, this->Exp );;
        default:
            return (this->A)*pow( (this->alpha)*second, this->Exp );
    }
}

double absff(double x)
{
    if (x < 0)
        return -x;
    return x;
}

double TransStringToDouble(string s)
{
    stringstream ss;
    ss << s;
    double result;
    ss >> result;
    return result;
}

string RemoveSpace(string s)
{
    unsigned i;
    for (i = 0; i < s.length(); i++)
        if (s[i] != ' ' && s[i] != 9 && s[i] != 13)	//9為水平tab,13為換行
            break;
    s = s.substr(i);
    return s;
}

void CIRCUIT::PutClockSource()
{
    GATE* gptr = new GATE("ClockSource", "PI");
    gptr->SetInTime(0);
    gptr->SetOutTime(0);
    PutGate(gptr);
}
string CIRCUIT::GetName(){   return name;    }
void CIRCUIT::PutWire( WIRE* w)
{
    wire_list.push_back(w);
    nametowire[w->GetName()] = w;
}
void CIRCUIT::PutGate( GATE* g )
{
    gate_list.push_back(g);
    nametogate[g->GetName()] = g;
}
WIRE* CIRCUIT::GetWire(int i)
{
    return wire_list[i];
}
WIRE* CIRCUIT::GetWire(string name)
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
GATE* CIRCUIT::GetGate( string name )
{
    if (nametogate.find(name) == nametogate.end())    return NULL;
    return nametogate[name];
}
GATE* CIRCUIT::GetGate( int i )
{
    return gate_list[i];
}

void CIRCUIT::ReadAgingData(){
    fstream file;
    file.open("./parameter/AgingRate.txt");
    
    for( int i = 0 ; i < 15 ; i++)
    {
        for( int k = 0 ; k < 4 ; k++ )  Rate[i][k] = 0 ;
    }
    for (int i = 1; i <=5; i++)
    {
        file >> Rate[i][0] >> Rate[i][1] >> Rate[i][2] >> Rate[i][3];
    }
    file >> Rate[10][0] >> Rate[10][1] >> Rate[10][2] >> Rate[10][3];
    for (int i = 6; i < 10; i++)
        for (int j = 0; j < 4;j++)
            Rate[i][j] = (Rate[10][j] - Rate[5][j]) / 5 * (i - 5) + Rate[5][j];
    file.close();
}
//////////////////////////////////////////////////////////////////
//             Read Netlist                                     //
//////////////////////////////////////////////////////////////////
void CIRCUIT::ReadCircuit()
{
    string vgname = "./benchmark/" + this->filename + ".vg"  ;
    printf( CYAN"[Info] Reading Circuit...\n");
    fstream file;
    file.open(vgname.c_str(),ios::in);
    
    if(!file){ cerr << "benchmark do not exist!\n"; return ;}
    char temp[1000]     ;
    bool cmt = false    ;
    int Nowmodule = -1  ;
    
    while (file.getline(temp, 1000))//一行多個命令無法處理
    {
        string temps = temp;
        
        if( cmt )
        {
            if (temps.find("*/") != string::npos)
                cmt = false;
            continue;
        }
        if (temps.find("/*") != string::npos){
            if (temps.find("*/") == string::npos)
                cmt = true;
            temps = temps.substr(0,temps.find("/*"));
        }
        if (temps.find("//") != string::npos)
            temps = temps.substr(0, temps.find("//"));
        temps = RemoveSpace(temps);
        if (temps.empty())
            continue;
        if( temps.find("endmodule") != string::npos)
        {
            continue;
        }
        else if (temps.find("assign")!=string::npos)
        {
            continue;
        }
        else if (temps.find("module")!=string::npos)
        {
            unsigned long st = temps.find("module") + 7;
            //CIRCUIT TC(temps.substr(st, temps.find(" (") - st));
            //Circuit.push_back(TC);
            this->setName(temps.substr(st, temps.find(" (") - st));
            
            Nowmodule++;
            while (file.getline(temp, 1000)){
                temps = temp;
                if (temps.find(")") != string::npos)
                    break;
            }
        }
        else if (temps.find("input")!=string::npos)
        {
            unsigned long st = temps.find("input") + 6;
            WIRE* w = new WIRE(temps.substr(st,temps.find(";") - st),PI);
            //Circuit[Nowmodule].PutWire(w);
            this->PutWire(w);
        }
        else if (temps.find("output") != string::npos)
        {
            unsigned long st = temps.find("output") + 7;
            WIRE* w = new WIRE(temps.substr(st, temps.find(";") - st), PO);
            //Circuit[Nowmodule].PutWire(w);
            this->PutWire(w);
            
        }
        else if (temps.find("wire") != string::npos)
        {
            unsigned long st = temps.find("wire") + 5;
            WIRE* w = new WIRE(temps.substr(st, temps.find(";") - st), INN);
            //Circuit[Nowmodule].PutWire(w);
            this->PutWire(w);
        }
        else
        {
            temps = RemoveSpace(temps);
            if (temps.empty())	continue;
            bool ok = false;
            for (int i = 0; i < Nowmodule; i++)
            {
                if ( this->GetName() == temps.substr(0, temps.find(" "))){
                    //可能再加入藉由之前讀入的module(非library)來建立gate,in/output的名稱不用記錄(在module內有),但要用以接到正確接口
                    ok = true;
                    break;
                }
            }
            if(!ok)//假設第一個為output,其它為input
            {
                string moduleN = temps.substr(0, temps.find(" "));	//module name,gate name,gate's output會寫在一行
                temps = temps.substr(temps.find(" ") + 1);
                string gateN = temps.substr(0, temps.find(" ("));
                temps = temps.substr(temps.find(" ") + 2);
                GATE* g = new GATE(gateN,moduleN);
                unsigned long st = temps.find("(") ;
                string ioN = temps.substr(st + 1, temps.find(")") - st - 1);
                g->SetOutput(this->GetWire(ioN));
                this->GetWire(ioN)->SetInput(g);
                temps = temps.substr(temps.find(")") + 1);
                
                while (file.getline(temp,1000))//gate's input 會在後面每行寫一個
                {
                    temps = temp;
                    st = temps.find("(");
                    string ioN = temps.substr(st + 1, temps.find(")") - st - 1);		//會有線接到1'b1,1'b0(常數)
                    g->SetInput(this->GetWire(ioN));
                    this->GetWire(ioN)->SetOutput(g);
                    //temps = temps.substr(temps.find(")") + 1);
                    if (temps.find(";") != string::npos)	break;
                }
                this->PutGate(g);
            }
        }
    }
    file.close();
    cout << wire_list.size() << endl ;
    cout << gate_list.size() << endl ;
    printf( CYAN"   ==> Finish Reading Circuit\n");
    return;
}
//////////////////////////////////////////////////////////////////
//          Read Timing Report(*.rpt)                           //
//////////////////////////////////////////////////////////////////
void CIRCUIT::ReadTimingReport( )
{
    string rptname = "./benchmark/" + filename + ".rpt"  ;
    printf( CYAN"[Info] Reading Cirtical Paths Info...\n") ;
    fstream file     ;
    string line      ;
    string sp        ;
    string ep        ;
    GATE*   gptr = NULL, *spptr = NULL, *epptr = NULL;
    PATH*   p = NULL;
    unsigned Path_No = 0;
    file.open( rptname.c_str(), ios::in);
    if( !file )
    {
        cerr << "Cannot opern rpt file" << endl ;
        return ;
    }
    
    
    while( getline(file, line) )
    {
        if( line.find("Startpoint") != string::npos )
        {
            if ( this->getPathALL().size() >= MAXPATHS )
                return;
            p = new PATH();
            sp = line.substr(line.find("Startpoint") + 12);
            sp = sp.substr(0, sp.find(" "));
            spptr = this->GetGate(sp);	//0為top-module
            if (spptr == NULL)	//起點為PI
                spptr = new GATE(sp,"PI");
        }
        else if (line.find("Endpoint") != string::npos)
        {
            ep = line.substr(line.find("Endpoint") + 10);
            ep = ep.substr(0, ep.find(" "));
            epptr = this->GetGate(ep);
            if (epptr == NULL)
                epptr = new GATE(ep, "PO");
        }
        
        if( line.find("---") == string::npos || sp == "")	continue;
        if( spptr->GetType() == "PI" && epptr->GetType() == "PO")
        {
            Path_No++;
            while (line.find("slack (MET)") == string::npos)	getline(file, line);
            continue;
        }
        getline(file, line);
        getline(file, line);//這2行的會反應在第一個gate時間上 不用記
        
        if( spptr->GetType() != "PI" )
        {
            while( getline(file, line) )
            {	//clock-source -> startpoint
                line = RemoveSpace(line);
                if (sp == line.substr(0, line.find("/")))	break;
                if (line.find("(net)") != string::npos)	continue;
                else if (line.find("(in)") != string::npos)
                {	//PI時間不計,如果有外部延遲後面分析再加入
                    spptr->SetClockPath(this->GetGate("ClockSource"));
                }//else if (line.find("(out)") != string::npos){}
                else
                {
                    string name = line.substr(0, line.find("/"));
                    double intime = TransStringToDouble(line.substr(line.find("&") + 1));
                    getline(file, line);
                    double outtime = TransStringToDouble(line.substr(line.find("&") + 1));
                    gptr = this->GetGate(name) ;
                    spptr->SetClockPath(gptr)       ;
                    gptr->SetInTime(intime)         ;
                    gptr->SetOutTime(outtime)       ;
                }
            }
        }
        if( spptr->GetType() == "PI" )//起點為PI的狀況
        {
            while (line.find("(in)") == string::npos)	getline(file, line);
            p->AddGate(spptr, 0, TransStringToDouble(line.substr(line.find("&") + 1)),0 );		//clock 到 起點的時間為0 (PI),tcq = 外部delay
            getline(file, line);
        }
        
        //-----------------Comb. Ckt (Maybe)--------------------//
        do
        {
            line = RemoveSpace(line);
            if (ep == line.substr(0, line.find("/")) || line.find("(out)")!=string::npos)	break;
            if (line.find("(net)") != string::npos)	continue;
            string name = line.substr(0, line.find("/"));
            double intime = TransStringToDouble(line.substr(line.find("&") + 1));
            getline(file, line);
            double outtime = TransStringToDouble(line.substr(line.find("&") + 1));
            gptr = this->GetGate(name);
            p->AddGate( gptr, intime, outtime, gptr->PV );
            
        } while ( getline(file, line) ) ;
        
        p->AddGate(epptr, TransStringToDouble(line.substr(line.find("&") + 1)), -1, 0 ) ; //arrival time
        
        getline(file, line);
        while (line.find("edge)") == string::npos)	getline(file, line);	//找clock [clock source] (rise/fall edge)
        if (period < 1)
            period = TransStringToDouble(line.substr(line.find("edge)") + 5));
        
        //在long path中 終點的時間都是加入了一個period的狀況 要減去 因為後面會改用別的Tc'
        if( epptr->GetType() == "PO" )
        {
            while (line.find("output external delay") == string::npos)	getline(file, line);
            double delay = TransStringToDouble(line.substr(line.find("-") + 1));
            p->SetCTE(0.0);
            p->SetST(delay);	//PO的setup time為外部delay
        }
        else
        {
            while (line.find("clock source latency") == string::npos) getline(file, line);
            
            while (getline(file, line))
            {
                line = RemoveSpace(line);
                if( ep == line.substr(0, line.find("/")) )
                {
                    double cte = TransStringToDouble(line.substr(line.find("&") + 1));
                    p->SetCTE(cte - period);		//這邊仍是有+1個period
                    break;
                }
                
                if( line.find("(net)") != string::npos )
                    continue ;
                else if( line.find("(in)") != string::npos )
                {
                    epptr->SetClockPath(this->GetGate("ClockSource"));
                }
                else
                {
                    string name = line.substr(0, line.find("/"));
                    double intime = TransStringToDouble(line.substr(line.find("&") + 1));
                    getline(file, line);
                    double outtime = TransStringToDouble(line.substr(line.find("&") + 1));
                    gptr = this->GetGate(name);
                    epptr->SetClockPath(gptr);
                    gptr->SetInTime(intime - period);		//檔案中為source -> ff的時間 + 1個period 需消去
                    gptr->SetOutTime(outtime - period);
                }
            }
            
            while (line.find("setup") == string::npos)	getline(file, line);
            double setup = TransStringToDouble(line.substr(line.find("-") + 1));
            p->SetST(setup);
        }
        spptr->Setflag();
        epptr->Setflag();
        p->SetType(LONG);
        p->SetNo(Path_No++);
        if (p->length()>2)		//中間有gate 不是直接連
            this->getPathALL().push_back(*p);
        sp = "";
    }
    printf( CYAN"   ==> Finish Reading Critical Paths info\n");
    printf( CYAN"   ==> Path count = %ld \n", getPathALL().size());
    file.close();
}

void CIRCUIT::ReadCpInfo( )//讀關聯性和迴歸線
{
    fstream file ;
    string cpname = "./benchmark/" +filename + ".cp" ;
    file.open( cpname.c_str()) ;
    if( !file )
    {
        printf(" Can't Open *.cp file\n");
    }
    map< unsigned, unsigned > mapping ;	//原編號(沒有去掉PI->PO & NO_GATE的) -> PathC的編號
    unsigned long ss = this->getPathCand().size() ;
    
    EdgeA = new double  *[ss]   ;
    EdgeB = new double  *[ss]   ;
    cor = new double    *[ss]   ;
    ser = new double    *[ss]   ;
    for( int i = 0 ; i < ss ; i++ )
    {
        EdgeA[i] = new double[ss];
        EdgeB[i] = new double[ss];
        cor[i] = new double[ss];
        ser[i] = new double[ss];
    }
    for(int i = 0; i < this->getPathCand().size(); i++)
    {
        mapping[this->getPathCand().at(i)->No()] = i ;
    }
    
    int     im = 0, jn = 0                  ;
    double  a = 0, b = 0, cc = 0, err = 0   ;
    string line  ;
    getline( file, line ) ;
    while( file >> im >> jn )
    {
        file >> line ;
        if (line == "nan")//unlimited slop rate
        {
            a = b = err = 10000;
            cc = 0;
            file >> line;
            file >> line;
            file >> line;
        }
        else
        {
            a = atof(line.c_str());
            file >> b;
            file >> line;
            if( line == "nan" )//y = b => 斜率為0 =>y之標準差為0 =>相關係數無窮大
                cc = 0;
            else
                cc = atof(line.c_str());
            file >> err;
        }
        if( mapping.find(im) == mapping.end() || mapping.find(jn) == mapping.end() )
            continue;
        int ii = mapping[im],jj = mapping[jn];
        
        EdgeA[ii][jj] = a   ;
        EdgeB[ii][jj] = b   ;
        cor[ii][jj] = cc    ;
        ser[ii][jj] = err   ;
    }
    file.close();
}

void CIRCUIT::CalPreInv(
               double x        ,//老化率 ex: x=0.14
               double &upper   ,//a推到b後，得到b的最大老化率
               double &lower   ,//a推到b後，得到b的最小老化率
               int a, int b, double aged_year
               )
{
    if( EdgeA[a][b] > 9999 )
    {
        upper = lower = 10000;
        return;
    }
    double dis = 1.96;	//+-多少個標準差 90% 1.65 ,95% 1.96 ,99% 2.58
    double y1 = EdgeA[a][b] * (x + 1) + EdgeB[a][b] * ((1 + AgingRate(WORST, aged_year)) / (1 + AgingRate(WORST, 10)));
    upper = y1 + ser[a][b] * dis* ((1 + AgingRate(WORST, aged_year)) / (1 + AgingRate(WORST, 10))) - 1   ;
    lower = y1 - ser[a][b] * dis* ((1 + AgingRate(WORST, aged_year)) / (1 + AgingRate(WORST, 10))) - 1   ;//按照比例調整
    //注意，此時y1,upper,lower單位是"老化率"，而非"年份"
    //cp檔是以"10"年為基準下做的，所以y=ax+b中的b需要微調
}

double CIRCUIT::CalPreAging( double x, int a, int b, double aged_year )//計算預測值，跟上面相比，直接計算y=ax+b，而不計算誤差
{
    if( EdgeA[a][b] > 9999 )
        return 10000;
    return EdgeA[a][b] * (x + 1) + EdgeB[a][b] * ((1 + AgingRate(WORST, aged_year)) / (1 + AgingRate(WORST, 10))) - 1;		//按照比例調整
}


////////////////////////////////////////////////////////////////////////////////////////////////
//          Cal Aging rate considering PV on Vth                                              //
//      Note: The unit of number in the function is V instead of mV                           //
////////////////////////////////////////////////////////////////////////////////////////////////
double CIRCUIT::CalAgingRateWithVthPV( double Vth_pv, double aged_year )
{
    double Vth_nbti = ( 1 - Sv[2]*Vth_pv )*(0.0039/2)*pow( (0.5*aged_year*365*86400), 0.2 ) ;
    double Vth_ttl = Vth_nbti + Vth_pv ;
    
    //printf("Vth_pv = %f, Agr = %f, gain = %f \n", Vth_pv, 2*Vth_nbti, 2*Vth_ttl);
    return 2*Vth_ttl ;
}

////////////////////////////////////////////////////////////////////////////////////////////////
//          Timing Checking(1)                                                                //
//      mode 0 : Exclude PV                                                                   //
//      mode 1 : Include PV                                                                   //
//      mode 3 : PV Verifcation(向右嚴格)                                                       //
//      mode 4 : PV Verifcation(向左嚴格)                                                       //
////////////////////////////////////////////////////////////////////////////////////////////////
bool CIRCUIT::Vio_Check( PATH* pptr, int stn, int edn, AGT ast, AGT aed, double year )
{
    GATE* stptr = pptr->Gate(0);
    GATE* edptr = pptr->Gate(pptr->length() - 1);
    
    //-----------------------------Clk_src~FF(head)-------------------------------------------------//
    double clks = 0.0   ;//Timing for Clk_src~FF(head)
    if( stptr->GetType() != "PI")
    {
        clks = pptr->GetCTH();//Fresh Clk_src~FF(head)
        double smallest = stptr->GetClockPath(1)->GetOutTime() - stptr->GetClockPath(1)->GetInTime();
        
        //--------------------Find Smallest Clk Buffer Self-Delay------------------------------------//
        for (int i = 2; i < stptr->ClockLength(); i++)
            if (stptr->GetClockPath(i)->GetOutTime() - stptr->GetClockPath(i)->GetInTime() < smallest)
                smallest = stptr->GetClockPath(i)->GetOutTime() - stptr->GetClockPath(i)->GetInTime();
        //smallest=最小的Clock Buffer timing
        //--------------------Clk_src~DCC間Buffer之Aging Timing--------------------------------------//
        for (int i = 0; i < stn; i++)
        {
            double delay = stptr->GetClockPath(i)->GetOutTime() - stptr->GetClockPath(i)->GetInTime() ;
            clks += ( delay )*AgingRate(DCC_NONE, year);
        }
        
        //--------------------DCC~FF(head)間Buffer之Aging Timing--------------------------------------//
        for (int i = stn; i < stptr->ClockLength(); i++)
        {
            double delay = stptr->GetClockPath(i)->GetOutTime() - stptr->GetClockPath(i)->GetInTime() ;
            clks += ( delay )*AgingRate(ast, year);
        }
        
        //--------------------DCC's Timing Delay-----------------------------------------------------//
        switch( ast )//DCC_type( left clk path )
        {
            case DCC_S:	//S為20%, M為40%, F為80% DCC
            case DCC_M:
                clks += smallest*( AgingRate(DCC_NONE, year ) + 1 )*1.33;
                break;
            case DCC_F:
                clks += smallest*( AgingRate(DCC_NONE, year) + 1 )*1.67;
                break;
            default:
                break;
        }
    }
    
    //-----------------------------Clk_src~FF(end)----------------------------------------------------//
    double clkt = 0.0 ;
    if( edptr->GetType() != "PO" )
    {
        clkt = pptr->GetCTE();//Original timing between Clk_src~FF(end)
        double smallest = edptr->GetClockPath(1)->GetOutTime() - edptr->GetClockPath(1)->GetInTime();
        
        //--------------------Find Smallest Clk Buffer Self Delay------------------------------------//
        for (int i = 2; i < edptr->ClockLength(); i++)
            if (edptr->GetClockPath(i)->GetOutTime() - edptr->GetClockPath(i)->GetInTime() < smallest)
                smallest = edptr->GetClockPath(i)->GetOutTime() - edptr->GetClockPath(i)->GetInTime();
        //--------------------Clk_src~DCC間Buffer之Aging Timing---------------------------------------//
        for (int i = 0; i < edn; i++)
        {
            double delay = edptr->GetClockPath(i)->GetOutTime() - edptr->GetClockPath(i)->GetInTime() ;
            clkt += ( delay )*AgingRate(DCC_NONE, year);
        }
        
        //--------------------DCC~FF(head)間Buffer之Aging Timing--------------------------------------//
        for (int i = edn; i < edptr->ClockLength(); i++)
        {
            double delay = edptr->GetClockPath(i)->GetOutTime() - edptr->GetClockPath(i)->GetInTime() ;
            clkt += ( delay )*AgingRate(aed, year);
        }
        
        //--------------------DCC's Timing Delay-----------------------------------------------------//
        switch(aed)//DCC_type( right clk path )
        {
            case DCC_S:
            case DCC_M:
                clkt += smallest*(AgingRate(DCC_NONE, year) + 1)*1.33;
                break;
            case DCC_F:
                clkt += smallest*(AgingRate(DCC_NONE, year) + 1)*1.67;
                break;
            default:
                break;
        }
    }
    
    //--------------------FF's Timing Delay(Plus Aging)------------------------------------------------//
    double Tcq = ( pptr->Out_time(0) - pptr->In_time(0) ) ;//FF's self delay(clk-->Q)
    if( stptr->GetType() != "PI" )
        Tcq *= ( AgingRate(FF, year) + 1.0) ;
    
    
    double DelayP = pptr->GetFreshDij() ; ;
    //##----------------- DelayP using different Aging Model -----------------------------------------
    DelayP += DelayP*AgingRate( NORMAL, year );
    
    //##------------------ Set/Hold Timing Constraint -----------------------------------------
    if( pptr->GetType() == LONG )
    {
        if (clks + Tcq + DelayP < clkt - pptr->GetST() + period )   { return true     ; }
        else                                                        { return false    ;}//Violate!
    }
    else
    {
        if( clks + Tcq + DelayP > clkt + pptr->GetHT())             return true     ;
        else                                                        return false    ;//Violate!
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////
//          Timing Checking(2)                                                                //
//      mode 0 : Exclude PV                                                                   //
//      mode 1 : Include PV                                                                   //
////////////////////////////////////////////////////////////////////////////////////////////////
bool CIRCUIT::Vio_Check( PATH* pptr, long double year, double Aging_P, bool mode  )
{
    
    GATE* stptr = pptr->Gate(0)                 ;
    GATE* edptr = pptr->Gate(pptr->length() - 1);
    int ls = stptr->ClockLength();
    int le = edptr->ClockLength();
    
    //-----------------------------Clk_src~FF(head/left)----------------------------------------------
    long double clks = 0.0;
    if( stptr->GetType() != "PI" )
    {
        clks = pptr->GetCTH() ;//Timing between Clk_src ~ FF(head) without aging and DCC.
        
        //--------------------Find Smallest Clk Buffer Self-Delay------------------------------------
        double smallest = stptr->GetClockPath(1)->GetOutTime() - stptr->GetClockPath(1)->GetInTime();
        for (int i = 2; i < stptr->ClockLength(); i++)
            if (stptr->GetClockPath(i)->GetOutTime() - stptr->GetClockPath(i)->GetInTime() < smallest)
                smallest = stptr->GetClockPath(i)->GetOutTime() - stptr->GetClockPath(i)->GetInTime();
        
        AGINGTYPE DCC_insert = DCC_NONE ;
        int i ;//clk buffer Position(Left).
        //--------------------Clk_src~DCC間Buffer之Aging Timing---------------------------------------
        for( i = 0; i < ls && stptr->GetClockPath(i)->GetDcc() == DCC_NONE; i++)
        {
            double delay = stptr->GetClockPath(i)->GetOutTime() - stptr->GetClockPath(i)->GetInTime() ;
            clks += ( delay )*AgingRate(DCC_NONE, year);
        }
        
        //--------------------DCC~FF(head)間Buffer之Aging Timing--------------------------------------
        if( i < ls )//if( DCC-placing position < left clk path length)
            DCC_insert = stptr->GetClockPath(i)->GetDcc();
        for( ; i < ls; i++ )
        {
            double delay = stptr->GetClockPath(i)->GetOutTime() - stptr->GetClockPath(i)->GetInTime() ;
            clks += ( delay )*AgingRate(DCC_insert, year);
        }
        
        //--------------------DCC's Self-Timing Delay-------------------------------------------------
        switch (DCC_insert)
        {
            case DCC_S:
            case DCC_M:
                clks += smallest*(AgingRate(DCC_NONE, year) + 1)*1.33;
                break;
            case DCC_F:
                clks += smallest*(AgingRate(DCC_NONE, year) + 1)*1.67;
                break;
            default:
                break;
        }
    }
    
    //-----------------------------Clk_src~FF(end/right)-------------------------------------------------//
    long double clkt = 0.0;
    if( edptr->GetType() != "PO" )
    {
        clkt = pptr->GetCTE() ;//timing betwn Clk_src ~ FF(end/right) without aging and DCC.
        //--------------------Find Smallest Clk Buffer Self-Delay----------------------------------------//
        double smallest = edptr->GetClockPath(1)->GetOutTime() - edptr->GetClockPath(1)->GetInTime() ;
        for (int i = 2; i < edptr->ClockLength(); i++)
            if (edptr->GetClockPath(i)->GetOutTime() - edptr->GetClockPath(i)->GetInTime() < smallest)
                smallest = edptr->GetClockPath(i)->GetOutTime() - edptr->GetClockPath(i)->GetInTime() ;
        
        int i ;//clk buffer Position(right).
        AGINGTYPE DCC_insert = DCC_NONE ;
        //--------------------Clk_src~DCC間Buffer之Aging+PV Timing-------------------------------------------//
        for( i = 0; i < le && edptr->GetClockPath(i)->GetDcc() == DCC_NONE; i++)
        {
            double delay = edptr->GetClockPath(i)->GetOutTime() - edptr->GetClockPath(i)->GetInTime() ;
            clkt += ( delay )*AgingRate(DCC_NONE, year);
        }
        if( i < le )
            DCC_insert = edptr->GetClockPath(i)->GetDcc();
        //--------------------DCC~FF(end/right)間Buffer之Aging+PV Timing-------------------------------------//
        for( ; i < le; i++ )
        {
            double delay = edptr->GetClockPath(i)->GetOutTime() - edptr->GetClockPath(i)->GetInTime() ;
            clkt += ( delay )*AgingRate(DCC_insert, year) ;
        }
        //--------------------DCC's Self-Timing Delay---------------------------------------------------//
        switch (DCC_insert)
        {
            case DCC_S:
            case DCC_M:
                clkt += smallest*(AgingRate(DCC_NONE, year) + 1)*1.33;
                break;
            case DCC_F:
                clkt += smallest*(AgingRate(DCC_NONE, year) + 1)*1.67;
                break;
            default:
                break;
        }
    }
    
    //--------------------FF's Timing Delay(Plus Aging)------------------------------------------------
    double Tcq = (pptr->Out_time(0) - pptr->In_time(0));
    if (stptr->GetType() != "PI")
        Tcq *= (AgingRate(FF, year) + 1.0);
    
    double Dij =  pptr->GetFreshDij() ;
    double DelayP = Dij ;
        //##----------------- DelayP using different Aging Model -----------------------------------------
    if( mode == 0 )
        DelayP += DelayP*Aging_P;
    else
    {
        double Vth_pv    = 0 ;
        double AgRate    = 0 ;
        double Gate_DN_Delay = 0 ;//Design
        double Gate_PV_Delay = 0 ;//PV
        for( int i = 0 ; i < pptr->length()-1 ; i++ )
        {
            Gate_DN_Delay = ( pptr->Out_time(i) - pptr->In_time(i) ) ;//Design delay
            Vth_pv = pptr->gTiming(i)->pv()                 ;
            Gate_PV_Delay = Gate_DN_Delay*( Vth_pv*2 )      ;
            AgRate = CalAgingRateWithVthPV( Vth_pv, year )  ;
            DelayP += ( Gate_DN_Delay + Gate_PV_Delay )*AgRate ;
        }
    }
    //##------------------ Set/Hold Timing Constraint -----------------------------------------
    if( pptr->GetType() == LONG )
    {
        if (clks + Tcq + DelayP < clkt - pptr->GetST() + period )   return true     ;
        else                                                        return false    ;//Violate!
    }
    else
    {
        if( clks + Tcq + DelayP > clkt + pptr->GetHT())             return true     ;
        else                                                        return false    ;//Violate!
    }
}


inline double absl(double x)
{
    if (x < 0 )
        return -x;
    return x;
}

double thershold = 0.8;	//R平方
double errlimit = 0.01;	//老化差(改名一下)

void CIRCUIT::AdjustConnect()//讀以上兩個值
{
    fstream ff;
    ff.open("./parameter/Parameter.txt", ios::in);
    string line;
    while (getline(ff, line))
    {
        if (line.find("thershold") != string::npos)
        {
            thershold = atof(line.c_str() + 9);
        }
        if (line.find("edge error") != string::npos)
        {
            errlimit = atof(line.c_str() + 10);
        }
    }
    ff.close();
}

bool CIRCUIT::Check_Connect(int a, int b,double year)
{
    if ( EdgeA[a][b] > 9999 || EdgeA[a][b]*EdgeA[a][b] < 0.000001 /* R^2 < 0.000001 */ )
        return false;
    if ( cor[a][b] < 0 )//負相關視為沒連接
        return false;
    if ( (cor[a][b]*cor[a][b]) /*R^2*/ < thershold )//相關係數要超過thershold才視為有邊
        return false;
    if ( absl( CalPreAging(AgingRate(WORST, year), a, b, year) - AgingRate(WORST, year)) > errlimit )
        return false;
    return true;
}

struct PN_W//是用來把path和計算的分數(選點時的)綁在一起做排序
{
    int pn ;//path number
    double w;//weight
    PN_W(int n, double ww) :pn(n), w(ww){}
};

bool PN_W_comp( PN_W a, PN_W b)//Used in sort(),which is STL func.
{
    if ( a.w > b.w ) return true  ;
    else             return false ;
}


double CIRCUIT::Overlap( int p //p:要考慮放到shortlist的號碼
//計算FF頭尾重疊率 => 取最大值=> 如何比較僅有頭/尾的? => 比較難成功 直接加一個值
//Used in ChooseVertexWithGreedyMDS()
//盡量起點端clk path放增加delay的DCC，終點端clk path放減少老化delay的DCC
//計算p這條path,跟其他shortlist path們的最大重疊率是多少
)
{
    double max = 0              ;
    PATH* pptr =  getPathCand().at(p)  ;
    GATE* stptr = pptr->Gate(0) ;//pptr->gate_list[0],where Path::vector<Gate*> gate_list ;
    GATE* edptr = pptr->Gate(pptr->length() - 1);
    
    
    for( int i = 0; i < getPathCand().size(); i++ )
    {
        //----------------Only Consider Pathes in shortlist-------------------------------------//
        if(!getPathCand().at(i)->Is_Chosen() || !getPathCand().at(i)->CheckAttack() ){   continue ;    }
        
        PATH* iptr = getPathCand().at(i) ;//shortlist path
        double score    ;
        double s1       ;//Path[p]的FF(head/left)之clkpath跟SlstPath[i]的FF(end/right)之clkpath間的重疊率
        double s2       ;//Path[p]的FF(end/right)之clkpath跟SlstPath[i]的FF(head/left)之clkpath間的重疊率
        
        //-----p path的FF(head/left)之clkpath 跟 i path(shortlist)的FF(end/right)之clkpath 間的重疊率-----//
        if( stptr->GetType() != "PI" )
        {
            GATE* gptr = iptr->Gate( iptr->length() - 1 );//Path[i]的FF(right/end),PO
            int c = 0 ;//index,Gate::vector<Gate*>clock_path[c],clk path中開始不一樣的位置(branch)
            while( c < stptr->ClockLength() && c < gptr->ClockLength() )
            {
                if( stptr->GetClockPath(c) != gptr->GetClockPath(c) )//gptr->Gate::clock_path[c]
                    break;
                c++;
            }
            s1 = (double)c / (double)stptr->ClockLength();
        }
        else{
            s1 = 1 ;//若只有一邊為FF,視為完全重疊(必比二邊都是FF的情況差)
        }
        
        //-----p path的FF(end/left)之clkpath 跟 i path(shortlist)的FF(head/left)之clkpath 間的重疊率-----//
        if( edptr->GetType() != "PO" )
        {
            GATE* gptr = iptr->Gate(0);
            int c = 0 ;//index in Gate::vector<Gate*>clock_path[c],開始不一樣的位置
            while( c < edptr->ClockLength() && c < gptr->ClockLength())
            {
                if( edptr->GetClockPath(c) != gptr->GetClockPath(c) ){  break ; }
                c++;
            }
            s2 = (double)c / (double)edptr->ClockLength();
        }
        else{
            s2 = 1 ;//若只有一邊為FF,視為完全重疊(必比二邊都是FF的情況差)
        }
        
        if( s1 > s2 )
            score = s1 * 2 + s2;//0.9+0.1 , 0.5+0.5兩種情形score分數一樣的情況
        else
            score = s2 * 2 + s1;
        
        if( max < score )
            max = score;
    }
    return max ;//重疊率越高,weight越低
}

double AtkPointRate( PATH* pptr )//計算自身的重疊率
{
    GATE* stptr = pptr->Gate(0);
    GATE* edptr = pptr->Gate(pptr->length() - 1);
    if( stptr->GetType() == "PI" || edptr->GetType() == "PO" ){
        return 0.75 ;
    }
    int c = 0 ;//index,Gate::vector<Gate*>clock_path[c],clk path中開始不一樣的位置(branch)
    while( c < stptr->ClockLength() && c < edptr->ClockLength())
    {
        if( stptr->GetClockPath(c) != edptr->GetClockPath(c) ){ break ;  }
        c++;
    }
    return (double)c * 2 / (double)(stptr->ClockLength() + edptr->ClockLength());
}


bool CIRCUIT::MDS( bool puthash  )//找shortlist, 回傳值true表示為domination set
{
    //--------------------- Store Info into Hash ------------------------------------------------------
    if( puthash )
    {
        this->_pHashTable->PutNowStatus( this );
        return true          ;
    }

    printf( YELLOW "[1] [MDS] \n" RESET );
    printf( YELLOW "Shortlist: " RESET );
    //--------------------- Var Declare ---------------------------------------------------------------
    int  *out_deg = new int[ this->getPathCand().size() ]   ;//white neighbors #
    int  *color   = new int[ this->getPathCand().size() ]   ;//black(-1),gray(0),white(1)
    int  cc       = 0                                       ;//shortlist size
    enum color{ black = -1 , gray = 0 , white = 1 };
    
    //--------------------- Graph Init ( color,degree of each node )-----------------------------------
    for( int i = 0; i < getPathCand().size() ; i++ )
    {
        getPathCand().at(i)->SetChoose(false) ;//Initialized as “None-shortlist”
        out_deg[i] =  0            ;
        if( !this->getPathCand().at(i)->GetCand() )
        {
            color[i] = black ;//Mine(black)
            continue         ;
        }
        color[i] = 1 ;//Candidate(white)
        for( int j = 0; j < this->getPathCand().size(); j++)//Calculate white vertex's outdegree
        {
            if( Check_Connect( i, j, year) && i != j && getPathCand().at(j)->GetCand() )
            {
                out_deg[i]++ ;
            }
        }
    }
    
    int mini            ;
    int wh_point        ;//white point #
    vector< PN_W > cand ;//PN:path number,W weight.PN_W is a struct.
    while( true )
    {   //Each iteration in this none-break loop consist of 2 procedures
        //1.target-making procedure,ex make vector cand
        //2.target-choosing procedure.
    
        //--------------------- Cal White Vertice # -------------------------------------------
        for( mini = 0, wh_point = 0; mini < getPathCand().size(); mini++)
        {
            if ( color[mini] == white )
                wh_point++;
        }
        //--------------------- All Vertice were dominated -----------------------------------
        if( wh_point == 0 )
        {
            printf("\n");
            return true;
        }
        cand.clear();
        
        //---------------------- Make vector<PN_W> cand(target)--------------------------------
        for( int i = 0; i < getPathCand().size() ; i++ )
        {
            //-------- Don't select mine/0-deg gray vertice -----------------------------------
            if( color[i] == black)
            {   continue;  }
            if( color[i] == gray && out_deg[i] == 0 )
            {   continue ; }
            
            //--------------------- Examinate Repetition --------------------------------------
            getPathCand().at(i)->SetChoose(true) ;//if the vertice is added into shortlist....
            if( this->_pHashTable->Exist( this ) )//...,then this permutation cause repetition against previous selection
            {
                getPathCand().at(i)->SetChoose(false);
                continue ;//if repetition occur, try next one.
            }
            getPathCand().at(i)->SetChoose(false) ;
            
            double w = 0 ;//weight
            w += (double)out_deg[i]/(double)wh_point;//加入i點後可減少的白點之比=i點degree/白點數目
            w -= 1*Overlap(i)                       ;//減掉此i點跟其它shortlist點的重疊率
            w -= 2*AtkPointRate(getPathCand().at(i))           ;//減掉自身的重疊率
            cand.push_back( PN_W( i, w ) )          ;
        }
        if( cand.size() == 0 )//找不到dominate set
        {
            return false;
        }
        
        //---------------------- Sort the target in increasing order ----------------------------
        sort( cand.begin(), cand.end(), PN_W_comp );
        
        //--------------------- Begin to select in this round -----------------------------------
        //--------------------- (1st round /other round) ----------------------------------------
        if( cc == 0 )//First round
        {
            int s = 0 ;//The sum of area of the targets.
                       //Each area of target is defined by their degree #.
            for( int i = 0; i < cand.size(); i++ )
            {
                s += out_deg[cand[i].pn];
            }
            int target;
            if( s > 0 )
                target = rand() % s ;//只有一個點時會出現 s = 0
            else
                target = 0;
            
            s = 0 ;
            for( int i = 0; i < cand.size(); i++)//計算飛鏢落在哪個標靶裡面
            {
                s += out_deg[cand[i].pn] ;
                if( s >= target )
                {
                    mini = cand[i].pn ;//此時mini是被飛鏢射中那個點(path)的id
                    break;
                }
            }
        }
        else//Later rounds
        {
            int ed = 0 ;//右界的味道//找degree最大者,如果相同就隨機
            while( ed < cand.size() )
            {
                if( absl(cand[ed].w - cand[0].w) < 0.5 )//0.5數值可變，tune出來的
                    ed++;
                else
                    break;
            }
            mini = cand[rand() % ed].pn;
        }
        
        //----------------------- After Select a Vertex, Update the Graph -----------------
        for( int nb = 0; nb < getPathCand().size() ; nb++ )
        {
            //nb/NB mean "NeighBor".
            if( mini == nb )	        continue ;//self.
            if( color[nb] == black )    continue ;//black vertex.
            
            //------------------- Find White Neighbors of selected vertex -----------------
            if( Check_Connect( mini, nb, year) && color[nb] == white )
            {
                for( int nbnb = 0; nbnb < getPathCand().size() ; nbnb++ )
                {
                    if( nb == nbnb ) continue ;
                    //----------- Find None-black NB of White NBs of selected vertex ------
                    if( Check_Connect(nbnb,nb,year) && color[nbnb] != black )
                    {   //Because nb will become gray, degree of nbnb(if connected to nb) should decrease by 1.
                        //The value of outdegree of a vertex only consider its white neighbors number.
                        out_deg[nbnb]--;
                    }
                }
                color[nb] = gray;//(white)NBs of selected vertex became "Gray".
            }
            if( Check_Connect(nb,mini,year) && color[mini] == white )
            {
                out_deg[nb]--;
            }
        }
        
        //-----------------------到此才真正的選點-----------------------------------------------
        getPathCand().at(mini)->SetChoose(true);
        out_deg[mini] = 0     ;
        color[mini]   = black ;
        cc++                  ;//shortlist size increase by 1.
        printf( "%d ", getPathCand().at(mini)->No() );
    }//while(true)
}

map< GATE*, int > cbuffer_code  ;//clock buffer
map< int, GATE* > cbuffer_decode;

int CIRCUIT::HashAllClockBuffer()//GenerateSAT中一開始就呼叫
{
    //1.只對Candidate/Mine Path做
    //2.每個clock souce上的buffer都會有對應的key id,且不重複
    //3.PI,PO是wire，不是flip-flop。所以沒clock path在其上
    //printf("進入 HashAllClockBuffer() \n");
    cbuffer_code.clear()    ;
    cbuffer_decode.clear()  ;
    int k = 0               ;//buffer num counter
    for( unsigned i = 0; i < this->getPathALL().size() ; i++ )
    {
        if( this->getPathALL()[i].IsSafe() )
        {
            continue ;
        }
        //-------------------Only en/decode buffers on the ClkPath of cand/mine------------------//
        PATH * pptr  = &this->getPathALL()[i] ;
        GATE * stptr = pptr->Gate(0)                  ;//PI,or FF(head/left)
        GATE * edptr = pptr->Gate(pptr->length() - 1) ;//PO,or FF(end/left)
        
        //-------------------把FF(head/left)'s clkpath上的buffer做encode/deode---------------------//
        if( stptr->GetType() != "PI" )
        {
            for( int j = 0 ; j < stptr->ClockLength() ; j++ )
                if( cbuffer_code.find(stptr->GetClockPath(j)) == cbuffer_code.end() )//沒找到
                {
                    cbuffer_code[stptr->GetClockPath(j)] = k  ;
                    cbuffer_decode[k] = stptr->GetClockPath(j);
                    k++ ;
                }
        }
        
        //-------------------把FF(end/right)'s clkpath上的buffer做encode/deode---------------------//
        if( edptr->GetType() != "PO" )
        {
            for (int j = 0 ; j < edptr->ClockLength(); j++ )
                if (cbuffer_code.find(edptr->GetClockPath(j)) == cbuffer_code.end())
                {
                    cbuffer_code[edptr->GetClockPath(j)] = k;
                    cbuffer_decode[k] = edptr->GetClockPath(j);
                    k++;
                }
        }
    }
    //printf("離開 HashAllClockBuffer() \n");
    return k ;//此時k=clock buffers總數
}

//------------------Classification of Mine/Candidate/Safe Pathes--------------------------------//
void CIRCUIT::PathClassify( )
{
    printf( CYAN"[Info] Classify paths into Mine/Cand/Safe...\n");
    double OriginalE = ERROR    ;
    this->period    = 0.0       ;
    
    //---------------------------- Set Period ---------------------------------------------------
    //Set such period such that original LT resides between 7~9 years.
    for( int i = 0; i < this->getPathALL().size(); i++ )
    {
        PATH* pptr = &(this->getPathALL().at(i));
        GATE* edptr = pptr->Gate(pptr->length() - 1);
        GATE* stptr = pptr->Gate(0)     ;
        pptr->SetPathID(i)              ;
        
        //-------------------------Non-Aging Timing----------------------------------------------
        double clks = pptr->GetCTH()    ;//Timing betwn clk ~ FF(head), without Aging.
        double clkt = pptr->GetCTE()    ;//Timing betwn clk ~ FF(end) , without Aging.
        double Tcq = pptr->Out_time(0) - pptr->In_time(0) ;//clk~Q(FF(head)), without Aging
        
        //-------------------------Plus Aging Timing of each Buffer on clock path----------------
        //Timing(right): clock --> FF(end) , clkt'(modfied) += clkt(original) + aging_timing(no DCC)
        for( int i = 0; i < edptr->ClockLength(); i++ )
        {
            double delay = edptr->GetClockPath(i)->GetOutTime() - edptr->GetClockPath(i)->GetInTime() ;
            clkt += ( delay )*AgingRate(DCC_NONE, year + PLUS) ;
        }
        //Timing(left): clock --> FF(head) , clks'(modfied) += clks(original) + aging_timing(no DCC)
        for( int i = 0; i < stptr->ClockLength(); i++ )
        {
            double delay = stptr->GetClockPath(i)->GetOutTime() - stptr->GetClockPath(i)->GetInTime() ;
            clks += ( delay )*AgingRate(DCC_NONE, year + PLUS)  ;
        }
        //-------------------------FF Timing with Aging------------------------------------------
        if( stptr->GetType() != "PI" )//PI是wire時，沒FF，自然沒Tcq
        {
            Tcq *= (1.0 + AgingRate(FF, static_cast<double>(year + PLUS)));
        }
        //-------------------------Period Timing with Aging---------------------------------------
        double Dij =  ( pptr->In_time(pptr->length() - 1) - pptr->Out_time(0) ) ;//沒老化,沒PV
        double pp = ( 1 + AgingRate(WORST, static_cast<double>(year + PLUS)))*( Dij ) + Tcq + (clks - clkt) + pptr->GetST();
        pp *= this->tight ;
        if( pp > this->period )
        {
            this->period = pp;
        }
    }
    //------------------------------ Cand/Mine/Ssfe --------------------------------------------
    //this->period = this->period + this->period*tc_mgn ;
    printf( CYAN"   ==> Path total size : " GREEN"%ld\n" RESET, this->getPathALL().size() );
    printf( CYAN"   ==> Clock Period    : " GREEN"%f\n" RESET, this->period );
    for( int i = 0; i < this->getPathALL().size(); i++ )
    {
        PATH* pptr = &this->getPathALL().at(i);
        pptr->SetAttack(false);
        pptr->SetSafe(true);
        GATE* stptr = pptr->Gate(0);
        GATE* edptr = pptr->Gate(pptr->length() - 1);
        int lst = stptr->ClockLength()  ;//length of clockpath(head)
        int led = edptr->ClockLength()  ;//length of clockpath(end)
        int branch = 0                  ;

        if( stptr->GetType() == "PI" )
        {
            for( int j = 0 ; j < led ; j++)//測end端clock path各個位置是否可放DCC?
            {
                for( int x = 0 ; x <= 3; x++ )//此位置三種DCC放放看會timing violation
                {
                    struct dccinfo *_pd = new struct dccinfo(0,j,DCC_NONE,(AGT)x ) ;
                    _pd->L_Name = "PI" ;
                    _pd->LGate  = NULL ;
                    _pd->R_Name = edptr->GetClockPath(j)->GetName() ;
                    _pd->RGate  = edptr->GetClockPath(j) ;
                    if( !Vio_Check(pptr,0,j,DCC_NONE,(AGT)x,year + OriginalE ) )
                    {
                        pptr->SetSafe(false)  ;
                        _pd->posfail = false  ;
                        if( Vio_Check( pptr,0,j,DCC_NONE,(AGT)x,year-OriginalE ) )
                        {
                            pptr->SetAttack(true)      ;
                            pptr->pldcc++              ;
                            _pd->middle = true         ;
                        }
                        else{
                            _pd->prefail = true ;
                        }
                    }
                    pptr->_vPDP.push_back(_pd) ;
                    pptr->_mapdcc[tuple<int,int,AGT,AGT>(0,j,DCC_NONE,(AGT)x)] = _pd ;
                }
            }
        }
        else if( edptr->GetType() == "PO" )
        {
            for( int j = 0 ; j < lst; j++)//測head端clock path各個位置是否可放DCC?
            {
                for( int x = 0 ; x <= 3; x++ )//此位置三種DCC放放看會timing violation?
                {
                    struct dccinfo *_pd = new struct dccinfo(j,0,(AGT)x,DCC_NONE ) ;
                    _pd->L_Name = stptr->GetClockPath(j)->GetName() ;
                    _pd->LGate  = stptr->GetClockPath(j) ;
                    _pd->R_Name = "PO" ;
                    _pd->RGate  = NULL ;
                    if( !Vio_Check(pptr,j,0,(AGT)x,DCC_NONE,year+OriginalE) )
                    {
                        pptr->SetSafe(false)   ;
                        _pd->posfail = false   ;
                        if( Vio_Check(pptr,j,0,(AGT)x,DCC_NONE,year-OriginalE ) )
                        {
                            pptr->SetAttack(true)      ;
                            pptr->pldcc++              ;
                            _pd->middle = true         ;
                        }
                        else{
                            _pd->prefail = true ;
                        }
                    }
                    pptr->_vPDP.push_back(_pd) ;
                    pptr->_mapdcc[tuple<int,int,AGT,AGT>(j,0,(AGINGTYPE)x,DCC_NONE)] = _pd ;
                }//x
            }//j
        }
        else//Path:FF~FF
        {
            while( branch < lst && branch < led  )
            {
                if( stptr->GetClockPath(branch) != edptr->GetClockPath(branch)) break ;
                for( int x = 0 ; x <= 3; x++ )
                {
                    struct dccinfo *_pd = new struct dccinfo(branch,branch,(AGT)x,(AGT)x ) ;
                    _pd->L_Name = stptr->GetClockPath(branch)->GetName() ;
                    _pd->LGate  = stptr->GetClockPath(branch) ;
                    _pd->R_Name = edptr->GetClockPath(branch)->GetName() ;
                    _pd->RGate  = edptr->GetClockPath(branch) ;
                    if( !Vio_Check(pptr,branch,branch,(AGT)x,(AGT)x,year+OriginalE ) )
                    {
                        pptr->SetSafe(false)  ;
                        _pd->posfail = false  ;
                        if( Vio_Check(pptr,branch,branch,(AGT)x,(AGT)x,year-OriginalE ) )
                        {
                            pptr->SetAttack(true)      ;
                            pptr->pldcc++              ;
                            _pd->middle = true         ;
                        }
                        else{
                            _pd->prefail = true     ;
                        }
                    }
                    pptr->_vPDP.push_back(_pd) ;
                    pptr->_mapdcc[tuple<int,int,AGT,AGT>(branch,branch,(AGT)x,(AGT)x)] = _pd ;
                }
                branch++;
            }//while( branch < lst && branch < led )
            //2.在處理分之岔開的兩邊。
            for( int j = branch; j < lst; j++ )//left
            {
                for( int k = branch; k < led; k++ )//right
                {
                    for( int x = 0; x <= 3; x++ )//left
                    {
                        for( int y = 0; y <= 3; y++ )//right
                        {
                            struct dccinfo *_pd = new struct dccinfo( j,k,(AGT)x,(AGT)y ) ;
                            _pd->L_Name = stptr->GetClockPath(j)->GetName() ;
                            _pd->LGate  = stptr->GetClockPath(j) ;
                            _pd->R_Name = edptr->GetClockPath(k)->GetName() ;
                            _pd->RGate  = edptr->GetClockPath(k) ;
                            if( !Vio_Check( pptr,j,k,(AGT)x,(AGT)y,year+OriginalE ) )
                            {
                                pptr->SetSafe(false)    ;
                                _pd->posfail = false    ;
                                if( Vio_Check(pptr,j,k,(AGT)x,(AGT)y,year-OriginalE ) )
                                {
                                    pptr->SetAttack(true)      ;
                                    pptr->pldcc++              ;
                                    _pd->middle = true         ;
                                }
                                else{
                                    _pd->prefail = true     ;
                                }
                            }
                            pptr->_vPDP.push_back(_pd) ;
                            pptr->_mapdcc[tuple<int,int,AGT,AGT>(j,k,(AGT)x,(AGT)y)] = _pd ;
                        }//y
                    }//x
                }//k
            }//j
        }//if-else
    }//for( this->getPathALL() )
    
    int aa = 0 ;//PI#
    int bb = 0 ;//PO#
    int cc = 0 ;//Cand#+Mine#
    int dd = 0 ;//Mine#
    
    for( unsigned i = 0 ; i < this->getPathALL().size(); i++ )
    {
        PATH* pptr = &this->getPathALL()[i]      ;
        GATE* stptr = pptr->Gate(0) ;
        GATE* edptr = pptr->Gate(pptr->length() - 1)    ;
        pptr->SetCand(false)        ;
        pptr->SetMine(false)        ;
        
        if( !pptr->IsSafe() )
        {
            if( stptr->GetType() == "PI")       aa++    ;
            
            else if (edptr->GetType() == "PO")  bb++    ;
            else                                cc++    ;
            this->getPathCand().push_back(pptr)                       ;
            if( !(pptr->CheckAttack()) )
            {
                dd++                ;
                pptr->SetMine(true) ;
            }
            else{
                pptr->SetCand(true) ;
            }
        }
    }
    if ( getPathCand().size() <= 0 )
    {
        printf( RED"    ==> [Warning] No Path Can Attack!\n" ) ;
    }
    if ( !CheckNoVio( year + PLUS ) )
    {
        printf( RED"    ==> [Warning] Too Tight Clock Period! \n" ) ;
    }
    printf( CYAN"   ==> Finish Classifing \n");
    return;
}

bool CIRCUIT::CheckNoVio( double LT /* = (year+PLUS) in main.cpp */ )//Called in main.cpp for early checking.
{
    printf( CYAN"[Info] Checking Violation... \n");
    for (int i = 0; i < this->getPathALL().size(); i++)
    {
        if ( !Vio_Check( &(this->getPathALL().at(i)), (long double)LT, AgingRate(WORST, year), 0 ) )
        {
            printf( "   ==> Path %d Violation! with %f\n", i, LT );
        }
    }
    printf( CYAN"   ==> " GREEN"No Timing Violation!\n" RESET);
    return true;
}

void CIRCUIT::GenerateSAT( )
{
    printf( "SAT Encoding ") ;
    chrono::steady_clock::time_point sat_st_time, sat_ed_time ;
    chrono::duration<double> SATTime ;
    
    fstream file ;
    
    string outputfile = "./CNF/sat.cnf" ;
    file.open( outputfile.c_str(), ios::out);
    map< GATE*, bool > exclusive;
    
    sat_st_time = chrono::steady_clock::now();
    //----------------En/Decode CLK Buffer that appear on Cand/Mine's CLK Path------------------------//
    HashAllClockBuffer() ;//每個clockbuffer之編號為在cbuffer_code內對應的號碼*2+1,*2+2
    
    //----------------CLK_Src上不能放DCC----------------------------------------------------------------//
    GATE* c_source = this->GetGate("ClockSource");
    file << '-' << cbuffer_code[c_source] * 2 + 1 << " 0" << endl;
    file << '-' << cbuffer_code[c_source] * 2 + 2 << " 0" << endl;
    
    //----------------Deal with Candidate/Mine Path----------------------------------------------------//
    for ( unsigned i = 0; i < getPathALL().size(); i++)
    {
        if ( getPathALL().at(i).IsSafe() )
        {
            continue ;
        }
        
        PATH* pptr =  &(getPathALL().at(i))      ;
        GATE* stptr = pptr->Gate(0) ;
        GATE* edptr = pptr->Gate(pptr->length()-1);
        int stn = 0, edn = 0 ;	//放置點(之後，包括自身都會受影響)
        int lst = stptr->ClockLength();
        int led = edptr->ClockLength();
        
        //----------------左Clk path不能有2個以上的DCC------------------------------//
        if( exclusive.find(stptr) == exclusive.end() /* 沒找到這個head Flip-Flop */)
        {
            for (int j = 0; j < lst; j++)
            {
                for (int k = j + 1; k < lst; k++)
                {
                    file << '-' << cbuffer_code[stptr->GetClockPath(j)] * 2 + 1 << ' ' << '-' << cbuffer_code[stptr->GetClockPath(k)] * 2 + 1 << " 0" << endl;
                    file << '-' << cbuffer_code[stptr->GetClockPath(j)] * 2 + 2 << ' ' << '-' << cbuffer_code[stptr->GetClockPath(k)] * 2 + 1 << " 0" << endl;
                    file << '-' << cbuffer_code[stptr->GetClockPath(j)] * 2 + 1 << ' ' << '-' << cbuffer_code[stptr->GetClockPath(k)] * 2 + 2 << " 0" << endl;
                    file << '-' << cbuffer_code[stptr->GetClockPath(j)] * 2 + 2 << ' ' << '-' << cbuffer_code[stptr->GetClockPath(k)] * 2 + 2 << " 0" << endl;
                }
            }
            exclusive[stptr] = true ;
            //exclusive[&FF(head)] = true ;
        }
        
        //----------------右Clk path不能有2個以上的DCC------------------------------//
        if( exclusive.find(edptr) == exclusive.end() /* 沒找到這個tail Flip-Flop */)
        {
            for( int j = 0; j < led; j++)
            {
                for( int k = j + 1; k < led; k++)
                {
                    file << '-' << cbuffer_code[edptr->GetClockPath(j)] * 2 + 1 << ' ' << '-' << cbuffer_code[edptr->GetClockPath(k)] * 2 + 1 << " 0" << endl;
                    file << '-' << cbuffer_code[edptr->GetClockPath(j)] * 2 + 2 << ' ' << '-' << cbuffer_code[edptr->GetClockPath(k)] * 2 + 1 << " 0" << endl;
                    file << '-' << cbuffer_code[edptr->GetClockPath(j)] * 2 + 1 << ' ' << '-' << cbuffer_code[edptr->GetClockPath(k)] * 2 + 2 << " 0" << endl;
                    file << '-' << cbuffer_code[edptr->GetClockPath(j)] * 2 + 2 << ' ' << '-' << cbuffer_code[edptr->GetClockPath(k)] * 2 + 2 << " 0" << endl;
                }
            }
            exclusive[edptr] = true;
            //exclusive[&FF(end)] = true ;
        }
        
        if( !( pptr->Is_Chosen() ) )//Non-shortlist Path can't Fail within n - e year.
        {
            if( stptr->type == "PI" )//Left is PI
            {
                for( edn = 0 ; edn < led; edn++ )
                {
                    for( int x = 0; x <= 3; x++) //代表差入的DCC型態, 無,20%, 80%, 40%
                    {
                        //if( !Vio_Check(pptr, 0, edn, DCC_NONE, (AGINGTYPE)x, year - SATError , mode2 ) )
                        auto mapitr = pptr->_mapdcc.find(tuple<int,int,AGT,AGT>(0,edn,DCC_NONE,(AGT)x)) ;
                        if( mapitr == pptr->_mapdcc.end() )
                        {
                            printf("發生矛盾 : PI %d , %d , %d , %d (保護)\n",0,edn,0,x) ;
                            return ;
                        }
                         
                        //if( !Vio_Check(pptr, 0, edn, DCC_NONE, (AGINGTYPE)x, year - SATError ,0,0,0 ) )
                        if( mapitr->second->prefail )//mine == prefail for this dcc placement
                        {
                            //一些省事的方法生對應的兩碼...
                            if ( x % 2 == 1)//x=2'b01(20%DCC),2'b11(40%DCC)--->右方那個bit皆為1
                                file << '-';
                            file << cbuffer_code[edptr->GetClockPath(edn)] * 2 + 1 << ' ';//針對右方的那一個bit
                            if (x >= 2)//x=2'b10(80%DCC),2'b11(40%DCC)--->左方那個bit皆為1
                                file << '-';
                            file << cbuffer_code[edptr->GetClockPath(edn)] * 2 + 2 << ' ';//針對左方的那一個bit
                            
                            for( int j = 0; j < led; j++ )//right clk path其他點都沒放DCC的情形
                            {
                                if (j == edn)	continue;
                                file << cbuffer_code[edptr->GetClockPath(j)] * 2 + 1 << ' ' << cbuffer_code[edptr->GetClockPath(j)] * 2 + 2 << ' ';
                            }
                            file << 0 << endl;
                        }
                    }
                }
            }
            else if ( edptr->GetType() == "PO" )//Right is PO
            {
                for( stn = 0 ; stn < lst; stn++)
                {
                    for (int x = 0; x <= 3; x++)
                    {
                        //if ( !Vio_Check(pptr, stn, 0, (AGINGTYPE)x, DCC_NONE, year - SATError, mode2 ))
                        
                        auto mapitr = pptr->_mapdcc.find(tuple<int,int,AGT,AGT>(stn,0,(AGT)x,DCC_NONE)) ;
                        if( mapitr == pptr->_mapdcc.end() )
                        {
                            printf("發生矛盾 : PO %d , %d , %d , %d (保護)\n",stn,0,x,stn) ;
                            return ;
                        }
                        
                        //if( mapitr->second->prefail )//mine == prefail for this dcc placement
                        //if ( !Vio_Check(pptr, stn, 0, (AGINGTYPE)x, DCC_NONE, year - SATError, 0,0,0 ))
                        if( mapitr->second->prefail )//mine == prefail for this dcc placement
                        {
                            if( x % 2 == 1 )//x=2'b01(20%DCC),2'b11(40%DCC)--->右方那個bit皆為1
                                file << '-';
                            file << cbuffer_code[stptr->GetClockPath(stn)] * 2 + 1 << ' ';//針對右方的那一個bit
                            if( x >= 2 )//x=2'b10(80%DCC),2'b11(40%DCC)
                                file << '-';
                            file << cbuffer_code[stptr->GetClockPath(stn)] * 2 + 2 << ' ';//針對左方的那一個bit
                            
                            for( int j = 0 ; j < lst ; j++ )
                            {
                                if (j == stn)	continue;
                                file << cbuffer_code[stptr->GetClockPath(j)] * 2 + 1 << ' ' << cbuffer_code[stptr->GetClockPath(j)] * 2 + 2 << ' ';
                            }
                            file << 0 << endl;
                        }
                    }
                }
            }
            else//The Non-Shortlist Path is FF ~ FF.
            {
                stn = edn = 0 ;//0(學長)
                while( stn < lst && edn < led )//放在共同區上
                {
                    if( stptr->GetClockPath(stn) != edptr->GetClockPath(edn))//遇到CLK Path的's Branch
                    { break ; }
                    
                    for( int x = 0; x <= 3; x++ )
                    {	//0 = 2'b00 NO DCC
                        //1 = 2'b01 DCC(20%),slow aging
                        //2 = 2'b10 DCC(80%),fast aging
                        //3 = 2'b11 DCC(40%)
                        //if( !Vio_Check( pptr, stn, edn, (AGINGTYPE)x, (AGINGTYPE)x, year - SATError, mode2 ) )
                        auto mapitr = pptr->_mapdcc.find(tuple<int,int,AGT,AGT>(stn,edn,(AGT)x,(AGT)x)) ;
                        if( mapitr == pptr->_mapdcc.end() )
                        {
                            printf("發生矛盾 : FF(共同) %d , %d , %d , %d (保護)\n",stn,edn,x,x) ;
                            return ;
                        }
                        
                        //if( !Vio_Check( pptr, stn, edn, (AGINGTYPE)x, (AGINGTYPE)x, year - SATError, 0,0,0 ) )
                        if( mapitr->second->prefail )
                        {
                            if (x % 2 == 1)
                                file << '-';
                            file << cbuffer_code[stptr->GetClockPath(stn)] * 2 + 1 << ' ';
                            if (x >= 2 )
                                file << '-';
                            file << cbuffer_code[stptr->GetClockPath(stn)] * 2 + 2 << ' ';
                            for (int j = 0; j < stptr->ClockLength(); j++){
                                if (j == stn)	continue;
                                file << cbuffer_code[stptr->GetClockPath(j)] * 2 + 1 << ' ' << cbuffer_code[stptr->GetClockPath(j)] * 2 + 2 << ' ';
                            }
                            for (int j = 0; j < edptr->ClockLength(); j++){
                                if (j == edn)	continue;
                                file << cbuffer_code[edptr->GetClockPath(j)] * 2 + 1 << ' ' << cbuffer_code[edptr->GetClockPath(j)] * 2 + 2 << ' ';
                            }
                            file << 0 << endl;
                        }
                    }
                    stn++;
                    edn++;
                }
                int b_point = stn;
                for( ; stn < lst; stn++)//放在非共同區
                {
                    for ( edn = b_point; edn < led; edn++ )
                    {
                        for( int x = 0; x <= 3; x++ )
                        {
                            for( int y = 0; y <= 3; y++ )
                            {
                                //if( !Vio_Check(pptr, stn, edn, (AGINGTYPE)x, (AGINGTYPE)y, year-SATError ,mode2 ) )
                                auto mapitr = pptr->_mapdcc.find(tuple<int,int,AGT,AGT>(stn,edn,(AGT)x,(AGT)y)) ;
                                if( mapitr == pptr->_mapdcc.end() )
                                {
                                    printf("發生矛盾 : FF(非共同) %d , %d , %d , %d (保護)\n",stn,edn,x,y) ;
                                    return ;
                                }
                                
                                //if( !Vio_Check(pptr, stn, edn, (AGINGTYPE)x, (AGINGTYPE)y, year-SATError ,0,0,0 ) )
                                if( mapitr->second->prefail )
                                {
                                    //if( mapitr->second->x != x || mapitr->second->y != y ) continue ;
                                    if (x % 2 == 1)	//01 11 -> 1[0] 0[0]
                                        file << '-';
                                    file << cbuffer_code[stptr->GetClockPath(stn)] * 2 + 1 << ' ';
                                    if (x >= 2)	//10 11
                                        file << '-';
                                    file << cbuffer_code[stptr->GetClockPath(stn)] * 2 + 2 << ' ';
                                    if (y % 2 == 1)
                                        file << '-';
                                    file << cbuffer_code[edptr->GetClockPath(edn)] * 2 + 1 << ' ';
                                    if (y >= 2)
                                        file << '-';
                                    file << cbuffer_code[edptr->GetClockPath(edn)] * 2 + 2 << ' ';
                                    for( int j = 0; j < stptr->ClockLength(); j++ )
                                    {
                                        if (j == stn)	continue;
                                        file << cbuffer_code[stptr->GetClockPath(j)] * 2 + 1 << ' ' << cbuffer_code[stptr->GetClockPath(j)] * 2 + 2 << ' ';
                                    }
                                    for( int j = 0; j < edptr->ClockLength(); j++)
                                    {
                                        if (j == edn)	continue;
                                        file << cbuffer_code[edptr->GetClockPath(j)] * 2 + 1 << ' ' << cbuffer_code[edptr->GetClockPath(j)] * 2 + 2 << ' ';
                                    }
                                    file << 0 << endl;
                                }
                            }
                        }
                    }
                }
            }
        }
        else//Shortlist Path's LifeTime Range from n-e to n+e.
        {
            
            if( stptr->GetType() == "PI" )
            {
                for( edn = 0 ; edn < led; edn++ )
                {
                    for( int x = 0 ; x <= 3 ; x++ )
                    {
                        auto mapitr = pptr->_mapdcc.find(tuple<int,int,AGT,AGT>(0,edn,DCC_NONE,(AGT)x)) ;
                        if( mapitr == pptr->_mapdcc.end() )
                        {
                            printf("發生矛盾 : PI %d , %d , %d , %d (攻擊)\n",0,edn,0,x) ;
                            return ;
                        }
        
                        //if ( Vio_Check(pptr, 0, edn, DCC_NONE, (AGINGTYPE)x, year + SATError , 0,0,0 )|| !Vio_Check(pptr, 0, edn, DCC_NONE, (AGINGTYPE)x, year - SATError, 0,0,0 ))
                        if( mapitr->second->posfail || mapitr->second->prefail )
                        {
                            if( x % 2 == 1 )
                                file << '-';
                            file << cbuffer_code[edptr->GetClockPath(edn)] * 2 + 1 << ' ';
                            if( x >= 2 )
                                file << '-';
                            file << cbuffer_code[edptr->GetClockPath(edn)] * 2 + 2 << ' ';
                            for( int j = 0; j < led; j++ )
                            {
                                if (j == edn)	continue;
                                file << cbuffer_code[edptr->GetClockPath(j)] * 2 + 1 << ' ' << cbuffer_code[edptr->GetClockPath(j)] * 2 + 2 << ' ';
                            }
                            file << 0 << endl;
                        }
                    }
                }
            }
            else if (edptr->GetType() == "PO")
            {
                for (stn = 0 ; stn < lst; stn++ )
                {
                    for (int x = 0 ; x <= 3 ; x++ )
                    {
                        auto mapitr = pptr->_mapdcc.find(tuple<int,int,AGT,AGT>(stn,0,(AGT)x,DCC_NONE ) ) ;
                        if( mapitr == pptr->_mapdcc.end() )
                        {
                            printf("發生矛盾 : PO %d , %d , %d , %d (攻擊)\n",stn,0,x,0) ;
                            return ;
                        }
                        
                        //if(Vio_Check(pptr, stn, 0, (AGINGTYPE)x, DCC_NONE, year + SATError , 0,0,0 ) || !Vio_Check(pptr, stn, 0, (AGINGTYPE)x, DCC_NONE, year - SATError ,0,0,0))
                        if( mapitr->second->posfail || mapitr->second->prefail )
                        {
                            if (x % 2 == 1)
                                file << '-';
                            file << cbuffer_code[stptr->GetClockPath(stn)] * 2 + 1 << ' ';
                            if (x >= 2)
                                file << '-';
                            file << cbuffer_code[stptr->GetClockPath(stn)] * 2 + 2 << ' ';
                            for (int j = 0; j < lst; j++){
                                if (j == stn)	continue;
                                file << cbuffer_code[stptr->GetClockPath(j)] * 2 + 1 << ' ' << cbuffer_code[stptr->GetClockPath(j)] * 2 + 2 << ' ';
                            }
                            file << 0 << endl;
                        }
                    }
                }
            }
            else
            {
                stn = edn = 0 ;//(0)
                while (stn < lst && edn < led)//放在共同區上
                {
                    if( stptr->GetClockPath(stn) != edptr->GetClockPath(edn) )
                        break;
                    for( int x = 0 ; x <= 3; x++ )
                    {
                        auto mapitr = pptr->_mapdcc.find(tuple<int,int,AGT,AGT>(stn,edn,(AGT)x,(AGT)x ) ) ;
                        if( mapitr == pptr->_mapdcc.end() )
                        {
                            printf("發生矛盾 : FF(共同) %d , %d , %d , %d (Shortlist防發散)\n",stn,edn,x,x) ;
                            return ;
                        }
                        
                        //if( Vio_Check(pptr, stn, edn, (AGINGTYPE)x, (AGINGTYPE)x, year + SATError ,0,0,0 ) || !Vio_Check(pptr, stn, edn, (AGINGTYPE)x, (AGINGTYPE)x, year - SATError ,0,0,0 ))
                        if( mapitr->second->posfail || mapitr->second->prefail )
                        {
                            if (x % 2 == 1)	//01 11 -> 10 00
                                file << '-';
                            file << cbuffer_code[stptr->GetClockPath(stn)] * 2 + 1 << ' ';
                            if (x >= 2)	//10 11
                                file << '-';
                            file << cbuffer_code[stptr->GetClockPath(stn)] * 2 + 2 << ' ';
                            for (int j = 0; j < stptr->ClockLength(); j++)
                            {
                                if (j == stn)	continue;
                                file << cbuffer_code[stptr->GetClockPath(j)] * 2 + 1 << ' ' << cbuffer_code[stptr->GetClockPath(j)] * 2 + 2 << ' ';
                            }
                            for (int j = 0; j < edptr->ClockLength(); j++){
                                if (j == edn)	continue;
                                file << cbuffer_code[edptr->GetClockPath(j)] * 2 + 1 << ' ' << cbuffer_code[edptr->GetClockPath(j)] * 2 + 2 << ' ';
                            }
                            file << 0 << endl;
                        }
                    }
                    stn++;
                    edn++;
                }
                int b_point = stn;
                for( ; stn < lst; stn++)
                {
                    for (edn = b_point; edn < led; edn++)
                    {
                        for (int x = 0; x <= 3; x++)
                        {
                            for (int y = 0; y <= 3; y++)
                            {
                                //if (Vio_Check(pptr, stn, edn, (AGINGTYPE)x, (AGINGTYPE)y, year + SATError , mode ) || !Vio_Check(pptr, stn, edn, (AGINGTYPE)x, (AGINGTYPE)y, year - SATError , mode ))
                                auto mapitr = pptr->_mapdcc.find(tuple<int,int,AGT,AGT>(stn,edn,(AGT)x,(AGT)y ) ) ;
                                if( mapitr == pptr->_mapdcc.end() )
                                {
                                    printf("發生矛盾 : FF(非共同) %d , %d , %d , %d (Shortlist防發散)\n",stn,edn,x,y) ;
                                    return ;
                                }
                                
                                //if (Vio_Check(pptr, stn, edn, (AGINGTYPE)x, (AGINGTYPE)y, year + SATError , 0,0,0 ) || !Vio_Check(pptr, stn, edn, (AGINGTYPE)x, (AGINGTYPE)y, year - SATError ,0,0,0 ))
                                if( mapitr->second->posfail || mapitr->second->prefail )
                                {
                                    if (x % 2 == 1)	//01 11 -> 1[0] 0[0]
                                        file << '-';
                                    file << cbuffer_code[stptr->GetClockPath(stn)] * 2 + 1 << ' ';
                                    if (x >= 2)	//10 11
                                        file << '-';
                                    file << cbuffer_code[stptr->GetClockPath(stn)] * 2 + 2 << ' ';
                                    if (y % 2 == 1)
                                        file << '-';
                                    file << cbuffer_code[edptr->GetClockPath(edn)] * 2 + 1 << ' ';
                                    if (y >= 2)
                                        file << '-';
                                    file << cbuffer_code[edptr->GetClockPath(edn)] * 2 + 2 << ' ';
                                    for (int j = 0; j < stptr->ClockLength(); j++){
                                        if (j == stn)	continue;
                                        file << cbuffer_code[stptr->GetClockPath(j)] * 2 + 1 << ' ' << cbuffer_code[stptr->GetClockPath(j)] * 2 + 2 << ' ';
                                    }
                                    for (int j = 0; j < edptr->ClockLength(); j++){
                                        if (j == edn)	continue;
                                        file << cbuffer_code[edptr->GetClockPath(j)] * 2 + 1 << ' ' << cbuffer_code[edptr->GetClockPath(j)] * 2 + 2 << ' ';
                                    }
                                    file << 0 << endl;
                                }
                            }
                        }
                    }
                }
            }
            
        }
        
    }
    sat_ed_time = chrono::steady_clock::now();
    SATTime = chrono::duration_cast<chrono::duration<double>>( sat_ed_time - sat_st_time );
    cout << "(" << CYAN << SATTime.count() << RESET << "s)" <<endl  ;
    file.close();
}

void CIRCUIT::printDCCLocation()
{
    
    system("./minisat ./CNF/best.cnf ./CNF/temp.sat 1> ./sat_report/minisat_std_output.txt 2> ./sat_report/minisat_warn_output.txt ");
    
    fstream file;
    file.open( "./CNF/temp.sat", ios::in );//temp.sat是minisat執行完的結果
    string line;
    getline( file, line );
    //--------------------- No Solution----- -------------------------------------
    int n1,n2;
    
    //--------------------- Decode & Put DCC -------------------------------------
    while( file >> n1 >> n2 )
    {
        if (n1 < 0 && n2 < 0)
        cbuffer_decode[(-n1 - 1) / 2]->SetDcc(DCC_NONE);
        else if(n1>0 && n2 < 0)
        cbuffer_decode[(n1 - 1) / 2]->SetDcc(DCC_S);//20% DCC
        else if(n1<0 && n2>0)
        cbuffer_decode[(-n1 - 1) / 2]->SetDcc(DCC_F);//80% DCC
        else
        cbuffer_decode[(n1 - 1) / 2]->SetDcc(DCC_M);//40% DCC
    }
    int cdcc = 0 ;
    _vDCCGate.clear() ;
    //--------------------- Show Result------- -------------------------------------
    printf( CYAN"----------------- DCC Placed Location --------------------\n" RESET);
    for( int i = 0; i < cbuffer_decode.size() ;i++ )
    {
        if ( cbuffer_decode[i]->GetDcc() != DCC_NONE )
        {
            ++cdcc ;
            cout << CYAN << cdcc << " : " BLUE<< cbuffer_decode[i]->GetName() << ' ' << cbuffer_decode[i]->GetDcc() << RESET << endl;
            _vDCCGate.push_back(cbuffer_decode[i]) ;
        }
    }
    printf( CYAN"------------------------------------------------------------\n" RESET);
}
int CIRCUIT::CallSatAndReadReport( int flag /*一般解or最佳解*/ )
{
    printf( "SAT Decoding ");
    chrono::steady_clock::time_point st_time, ed_time ;
    chrono::duration<double> callsattime ;
    st_time   = chrono::steady_clock::now( ) ;
    //----------- Init [Don't Put DCC]-----------------------------------------
    for (int i = 0; i < this->getPathALL().size(); i++)
    {
        GATE* stptr = this->getPathALL().at(i).Gate(0);
        GATE* edptr = this->getPathALL().at(i).Gate( this->getPathALL().at(i).length() - 1 ) ;
        //------------ Left CLK Path Init--------------------------------------
        for( int j = 0; j < stptr->ClockLength(); j++ )
            stptr->GetClockPath(j)->SetDcc(DCC_NONE);
        //------------ Right CLK Path Init-------------------------------------
        for( int j = 0; j < edptr->ClockLength(); j++ )
            edptr->GetClockPath(j)->SetDcc(DCC_NONE);
    }
    
    //--------------------- Call Minisat ----------------------------------------
    if( flag == 1 )
        system("./minisat ./CNF/best.cnf ./CNF/temp.sat 1> ./sat_report/minisat_std_output.txt 2> ./sat_report/minisat_warn_output.txt ");
    else
        system("./minisat ./CNF/sat.cnf ./CNF/temp.sat 1> ./sat_report/minisat_std_output.txt 2> ./sat_report/minisat_warn_output.txt ");
    
    fstream file;
    file.open( "./CNF/temp.sat", ios::in );//temp.sat是minisat執行完的結果
    string line;
    getline( file, line );
    //--------------------- No Solution---------------------------------------------
    if( line.find("UNSAT")!=string::npos )
    {
        ed_time     = chrono::steady_clock::now( ) ;
        callsattime = chrono::duration_cast<chrono::duration<double>>( ed_time - st_time )    ;
        cout << "(" << CYAN << callsattime.count() << RESET << "s)"<<endl ;
        return 0 ;
    }
    
    int n1,n2;
    //--------------------- Decode & [Put DCC] -------------------------------------
    while( file >> n1 >> n2 )
    {
        if (n1 < 0 && n2 < 0)
            cbuffer_decode[(-n1 - 1) / 2]->SetDcc(DCC_NONE);
        else if(n1>0 && n2 < 0)
            cbuffer_decode[(n1 - 1) / 2]->SetDcc(DCC_S);//20% DCC
        else if(n1<0 && n2>0)
            cbuffer_decode[(-n1 - 1) / 2]->SetDcc(DCC_F);//80% DCC
        else
            cbuffer_decode[(n1 - 1) / 2]->SetDcc(DCC_M);//40% DCC
    }
    file.close() ;
    
    //--------------------- Cal SAT Time ------------------------------------------
    ed_time     = chrono::steady_clock::now( ) ;
    callsattime = chrono::duration_cast<chrono::duration<double>>( ed_time - st_time )    ;
    cout << "(" << CYAN << callsattime.count() << RESET << "s)"<<endl ;
    
    //--------------------- Show Result------- -------------------------------------
    int cdcc = 0 ;
    _vDCCGate.clear() ;
    for( int i = 0; i < cbuffer_decode.size() ;i++ ){
        if ( cbuffer_decode[i]->GetDcc() != DCC_NONE ){
            ++cdcc ;
        }
    }
    return cdcc;
}

void CIRCUIT::CheckOriLifeTime()
{							//有可能決定Tc的path不在candidate中(mine裡)
    printf( CYAN"[Info] Check Original Lifetime...\n" );		//為何會有>10? 較後面的Path slack大很多(自身lifetime長) 且和前面cp的關連低(前端老化不足)
    double up = 10.0, low = 0.0;					//為何會有接近7? 1.Path之間的slack都接近=>不管老化哪個都不會太差 2.path之間相關度都高

    for (int i = 0; i < getPathCand().size(); i++)          //不同candidate會造成此處不同... 取聯集
    {
        if (!getPathCand().at(i)->CheckAttack())
            continue;
        double e_upper = 10000, e_lower = 10000;
        //getPathCand().at(i) --> Path[j]
        for( int j = 0; j < getPathCand().size(); j++ )
        {
            
            if( EdgeA[i][j] > 9999 )
                continue ;
            double st = 1.0, ed = 50.0, mid = 0 ;
            while (ed - st > 0.0001)
            {
                mid = (st + ed) / 2;
                double upper, lower;
                CalPreInv(AgingRate(WORST, mid), upper, lower, i, j, mid);				//y = ax+b => 分成lower bound/upper bound去求最遠能差多少
                double Aging_P;

                if (upper > AgingRate(WORST, mid))
                    Aging_P = AgingRate(WORST, mid);
                else if (upper < AgingRate(BEST, mid))
                    Aging_P = AgingRate(BEST, mid);
                else
                    Aging_P = upper;
                if (Vio_Check(getPathCand().at(j), (long double)mid, Aging_P,0))
                    st = mid;
                else
                    ed = mid;
            }
    
            if( mid < e_upper )
                e_upper = mid;				//最早的點(因為發生錯誤最早在此時)
            st = 1.0; ed = 50.0;
            while( ed - st > 0.0001 )
            {
                mid = (st + ed) / 2;
                double upper, lower;
                CalPreInv( AgingRate(WORST, mid), upper, lower, i, j, mid );
                double Aging_P ;
                if (lower > AgingRate(WORST, mid))
                    Aging_P = AgingRate(WORST, mid);
                else if (lower < AgingRate(BEST, mid))
                    Aging_P = AgingRate(BEST, mid);
                else
                    Aging_P = lower;
                if (Vio_Check(getPathCand().at(j), (long double)mid, Aging_P,0))
                    st = mid;
                else
                    ed = mid;
            }
            if (mid < e_lower)
                e_lower = mid;
        }
        if (up > e_upper)
            up = e_upper;
        if (low < e_lower)
            low = e_lower;
        
    }
    printf( CYAN"   ==> " GREEN"%f " CYAN"~ " GREEN"%f \n", up, low );
}


double CIRCUIT::CalQuality( double &up, double &low , int mode )
{
    printf("Calculate LT ");
    chrono::steady_clock::time_point q_st_time, q_ed_time ;
    chrono::duration<double> qtime ;
    q_st_time = chrono::steady_clock::now();
    
    up = 10.0 ; low = 0.0 ;
    for( int i = 0; i < getPathCand().size(); i++ )
    {
        if( !getPathCand().at(i)->CheckAttack() )  continue ;
    
        double e_upper = 10000, e_lower = 10000 ;
        for (int j = 0; j < getPathCand().size(); j++)
        {
            if( EdgeA[i][j] > 9999 )   continue;
            double st = 0.0 , ed = 10.0, mid = 0 ;
            while( ed - st > 0.001 )
            {
                mid = (st + ed) / 2;
                double upper , lower ;
                CalPreInv( AgingRate(WORST, mid), upper, lower, i , j, mid ) ;
                double Aging_P ;
                if( upper > AgingRate( WORST, mid ) )
                    Aging_P = AgingRate( WORST, mid ) ;
                else
                    Aging_P = upper;
                
                if( Vio_Check(getPathCand().at(j), (long double)mid, Aging_P,0) )
                {
                    st = mid ;//可存活
                }
                else
                {  ed = mid ; }
            }
            if( mid < e_upper )
            {
                e_upper = mid  ;
            }
            //ppt:最後因為所以的interval同時存在, 最早的值就是電路發生錯誤的時間 所以取各path最小的upper, lower bound做為整體的upper/lower bound
            
            st = 0.0 ; ed = 10.0   ;
            while ( ed - st > 0.001 )
            {
                mid = (st + ed) / 2;
                double upper, lower;
                CalPreInv(AgingRate(WORST, mid), upper, lower, i, j, mid );
                double Aging_P;
                if( lower > AgingRate(WORST, mid) )
                    Aging_P = AgingRate(WORST, mid );
                else
                    Aging_P = lower;
                if( Vio_Check(getPathCand().at(j), (long double)mid, Aging_P,0))
                    st = mid;
                else
                    ed = mid;
            }
            if( mid < e_lower )
            {
                e_lower = mid ;
            }
        }//for j
        
        //一個path出發點，就是一個operation load
        //每個operation load都會發生 所以取他們的聯集為總體的lifetime
        if( up > e_upper )
        {
            up = e_upper ;
        }
        if( low < e_lower)
        {
            low = e_lower;
        }
    }//fot i
    
    q_ed_time = chrono::steady_clock::now();
    qtime = chrono::duration_cast<chrono::duration<double>>( q_ed_time - q_st_time );
    cout << "(" << CYAN << qtime.count() << RESET << "s)"<<endl ;
    return 0.0 ;
}
double CIRCUIT::CalPathAginRateWithPV( PATH * pptr, double year )
{
    double Dij    = pptr->GetFreshDij() ;
    double Dij_pv = Dij ;
    double Vth_pv = 0   ;
    double Delta  = 0   ;

    for( int i = 1 ; i < pptr->length()-1 ; i++ )
    {
        Vth_pv = pptr->gTiming(i)->pv() ;
        Delta = ( pptr->Out_time(i) - pptr->In_time(i) )*CalAgingRateWithVthPV( Vth_pv, year ) ;
        Dij_pv += Delta ;
    }
    return Dij_pv/Dij - 1 ;
}


bool CheckImpact( PATH* pptr )//此path的頭尾FFs，這兩個FF的clock path若有放DCC就回傳true, and vice versa.
{
    GATE* gptr;
    gptr = pptr->Gate(0) ;
    if ( gptr->GetType() != "PI")
    {
        for (int i = 0; i < gptr->ClockLength();i++)
            if( gptr->GetClockPath(i)->GetDcc() != DCC_NONE )
                return true;
    }
    gptr = pptr->Gate( pptr->length() - 1);
    if (gptr->GetType() != "PO")
    {
        for (int i = 0; i < gptr->ClockLength(); i++)
            if (gptr->GetClockPath(i)->GetDcc() != DCC_NONE)
                return true;
    }
    return false;
}

void CIRCUIT::RemoveRDCCs()
{
    map< GATE*, bool > must;
    printf( YELLOW "---------------------------------------------\n" RESET );
    printf( YELLOW"[2] [Remove Spare DCCs]\n" RESET )      ;
    //--------Mark DCC positions that is on Shortlist paths' clk path---------------------
    for ( int i = 0; i < getPathCand().size(); i++ )
    {
        if ( !getPathCand().at(i)->Is_Chosen() )//若此pah非在shortlist中，則不做
            continue ;
        PATH* pptr = getPathCand().at(i) ;
        GATE* stptr = pptr->Gate(0) ;//FF(head)
        GATE* edptr = pptr->Gate( pptr->length() - 1 ) ;//FF(end)
        
        //---- Left CLK Path --------------------------------------
        for( int j = 0; j < stptr->ClockLength(); j++)
        {
            if ( stptr->GetClockPath(j)->GetDcc() != DCC_NONE )
                must[stptr->GetClockPath(j)] = true;
        }
        //---- Right CLK Path --------------------------------------
        for( int j = 0; j < edptr->ClockLength(); j++)
        {
            if (edptr->GetClockPath(j)->GetDcc()!= DCC_NONE )
                must[edptr->GetClockPath(j)] = true;
        }
    }
    fstream file;
    file.open( "./CNF/sat.cnf", ios::out | ios::app ) ;
    
    //-----Find additional DCC placement that is not on shortlist paths' clk path-----------//
    for( int i = 0; i < this->getPathALL().size(); i++)//unsafe的點有可能被放無關緊要的DCC(只要不會過早)
    {
        if( this->getPathALL()[i].Is_Chosen() || this->getPathALL()[i].IsSafe() ) continue ;
        
        PATH* pptr = &this->getPathALL()[i];
        GATE* stptr = pptr->Gate(0);
        GATE* edptr = pptr->Gate(pptr->length() - 1);
        bool flag = true;
        //以下不處理shortlist,跟safe的path
        
        //FF(head)'s clk path:
        for( int j = 0; j < stptr->ClockLength(); j++ )
        {
            //must.find() == must.end()代表沒找到
            if(stptr->GetClockPath(j)->GetDcc() != DCC_NONE /*有放Dcc*/ && must.find(stptr->GetClockPath(j)) != must.end() /*有找到*/)
            {
                flag = false;//不能移掉dcc(因為此DCC在shortlist's path上)
                break ;//換下一條path(一個clk path只能放一個dcc，所以檢查下去沒意義)
            }
        }
        //FF(end)'s clk path:
        for(int j = 0; j < edptr->ClockLength() && flag ; j++)
        {
            //此Gate有放DCC,且此DCC所放置的點，也在其他沒選中的C或mine的clock path上
            if (edptr->GetClockPath(j)->GetDcc() != DCC_NONE && must.find(edptr->GetClockPath(j)) != must.end())
            {
                flag = false;//不能移掉dcc(因為此DCC在shortlist's path上)
                break;//換下一條path(一個clk path只能放一個dcc，所以檢查下去沒意義)
            }
        }
        if( !flag ) continue ;
        
        //---------------------GenerateSAT(App)---------------------------------------//
        for (int j = 0; j < stptr->ClockLength(); j++)
        {
            //global:map< Gate * , int > cbuffer_code ;
            file << '-' << cbuffer_code[stptr->GetClockPath(j)] * 2 + 1 << " 0" << endl;
            file << '-' << cbuffer_code[stptr->GetClockPath(j)] * 2 + 2 << " 0" << endl;
        }
        for (int j = 0; j < edptr->ClockLength(); j++)
        {
            file << '-' << cbuffer_code[edptr->GetClockPath(j)] * 2 + 1 << " 0" << endl;
            file << '-' << cbuffer_code[edptr->GetClockPath(j)] * 2 + 2 << " 0" << endl;
        }
        
    }
    file.close();
}


bool AnotherSol()//將結果反向
{
    fstream file    ;
    fstream solution;
    string  line    ;
    int     dccno   ;
    file.open("./CNF/sat.cnf", ios::out | ios::app );
    solution.open("./CNF/temp.sat", ios::in );
    if( !file )     {   printf( "fail to open sat.cnf \n" ) ; }
    if( !solution ) {   printf( "fail to open temp.sat\n" ) ; }
    
    getline( solution, line ) ;
    if( line.find("UNSAT") != string::npos ){   return false; }
    printf("SAT Encoding ");
    chrono::steady_clock::time_point q_st_time, q_ed_time ;
    chrono::duration<double> qtime ;
    q_st_time = chrono::steady_clock::now();
    while ( solution >> dccno)//將原有的解反向後，放回(ios:app)進'sat.cnf'
    {
        file << -dccno << ' ';
    }
    //--------- Timing ------------------------------
    q_ed_time = chrono::steady_clock::now();
    qtime = chrono::duration_cast<chrono::duration<double>>( q_ed_time - q_st_time );
    cout << "(" << CYAN << qtime.count() << RESET << "s)" << endl  ;
    
    //---------- File Close --------------------------
    file << endl;
    file.close();
    solution.close();
    return true;
}
unsigned HASHTABLE::CalKey( CIRCUIT *circuit )
{
    unsigned key = 0x0  ;//32-bit
    unsigned temp = 0x0 ;//32-bit
    for( int i = 0; i < circuit->getPathCand().size(); i++ )
    {
        temp <<= 1;
        if( circuit->getPathCand().at(i)->Is_Chosen() )
            temp++;
        if( (i+1)%size == 0 )
        {
            key ^= temp;//XOR
            temp = 0x0;
        }
    }
    key ^= temp;//XOR
    return key;
}
bool HASHTABLE::Exist( CIRCUIT *circuit )
{
    unsigned key = CalKey( circuit );
    if( !exist[key] ){  return false ; }
    
    for( int i = 0; i < circuit->getPathCand().size(); i++)//可能有不同的狀態對應到同一種key，所以要一一比對
    {
        if( circuit->getPathCand().at(i)->Is_Chosen() != choose[key][i] ){  return false ;  }
    }
    return true;
}
void HASHTABLE::PutNowStatus( CIRCUIT *circuit )
{
    unsigned key = CalKey( circuit );
    exist[key] = true;
    for( int i = 0; i < circuit->getPathCand().size(); i++ )
    {
        choose[key][i] = circuit->getPathCand().at(i)->Is_Chosen() ;//若重複就覆蓋
    }
}





double CDF( double x, double m, double s )
{
    
    //double cdf = 0.5*( 1 + erf( (year - avg)/(sd*sqrt(2)) ) );
    double cdf = 0.5 * erfc(-(x-m)/(s*sqrt(2))) ;
    return cdf ;
}
void CIRCUIT::InstanceProab( )
{
    double UB = 0, LB = 0 ;
    if( this->year == 3 ){ LB = 1 ; UB = 5 ; }
    if( this->year == 4 ){ LB = 2 ; UB = 6 ; }
    if( this->year == 5 ){ LB = 3 ; UB = 7 ; }
    double range= ( UB - LB )/(100)        ;
    
    
    int    *vctrL= new int [ 100 ]         ;
    int    *vctrU= new int [ 100 ]         ;
    double *vprob= new double [ PVtimes ]  ;
    
    
    //---- Initialization ----------------------------
    for( int i = 0 ; i < 100 ; i++ ){ vctrL[i] = vctrU[i] = 0 ; }
    
    int index = 0 ;
    for( int i = 0 ; i < this->getInstLT().size() ; i++  )
    {
        index = (this->getInstLT().at(i).first  - LB)/range ;
        vctrL[index] =vctrL[index] + 1 ;
        index = (this->getInstLT().at(i).second - LB)/range ;
        vctrU[index] =vctrU[index] + 1 ;
    }
    
    double avg_prob = 0 ;
    double mean = (UB+LB)/2 ;
    
    for( int i = 0 ; i < this->getInstLT().size() ; i++  )
    {
        double avg_ins = ( getInstLT().at(i).second + getInstLT().at(i).first )/2 ;
        double sd_ins  = ( getInstLT().at(i).second - getInstLT().at(i).first )*0.95/6;
        double cdf1 =  CDF( mean*1.1, avg_ins, sd_ins ) ;
        double cdf2 =  CDF( mean*0.9, avg_ins, sd_ins ) ;
        vprob[i] = cdf1 - cdf2 ;
        avg_prob += vprob[i] ;
    }
    //---- Output File -----------------------------------
    string file_dist_name = "./quality/" + this->filename + "_" + to_string( (int)(this->year) ) + "_" + to_string( (int)(this->PVRange*1000) ) + "mV_dist.txt " ;
    string file_inst_name = "./quality/" + this->filename + "_" + to_string( (int)(this->year) ) + "_" + to_string( (int)(this->PVRange*1000) ) + "mV_inst.txt " ;
    FILE *foutput1 = fopen( file_dist_name.c_str(), "w+t");
    FILE *foutput2 = fopen( file_inst_name.c_str(), "w+t");
    for( int i = 0 ; i < 100 ; i++ )
    {
        fprintf( foutput1, "%f \t %d %d\n", LB + i*range, vctrL[i], vctrU[i] );
    }
    for( int i = 0 ; i < getInstLT().size()  ; i++ )
    {
        fprintf( foutput2, "Instance(%d): \t %f %f %f \n", i, getInstLT().at(i).first, getInstLT().at(i).second, vprob[i] );
    }
    //---- Output Screen -----------------------------------
    avg_prob /= ( getInstLT().size() ) ;
    cout << CYAN << "[Setting] granularity     = " << GRN << range         << " year" << RESET << endl ;
    cout << CYAN << "[Setting] Mean            = " << GRN << mean          << " year" << RESET << endl ;
    cout << CYAN << "[Result]  Arithmetic Avg. = " << GRN << avg_prob*100  << "\%"   << RESET << endl ;
    
    
}
