#include"aging.h"
#include<math.h>
#include<fstream>
#include<iostream>

using namespace std;

double Rate[15][4];
double a, alpha, n;

void ReadAgingData(){
    fstream file;
    file.open("AgingRate.txt");
    
    for( int i = 0 ; i < 15 ; i++)
    {
        for( int k = 0 ; k < 4 ; k++ )
        {
            Rate[i][k] = 0 ;
        }
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
    file.open("Parameter.txt");
    file >> a >> alpha >> n;
    file.close();
}

double AgingRate(AGINGTYPE status, double year)
{
    
    double second = year * 365 * 86400;	//0.0039*(0.5*t)^0.2 + 1	average case -> worst caseßY•h±º0.5(alpha)
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
                return a*pow(alpha*second, n);
            case NORMAL:
                return a*pow(alpha*second, n);
            case BEST:
                return a*pow(0.25*second, n);
            default:
                return a*pow(alpha*second, n);
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
            return a*pow(alpha*second, n);
        case NORMAL:		//º»Æ…ß‚NORMAL©MWORSTßÔ¶®¨€¶P
            return a*pow(alpha*second, n);
        case BEST:
            return a*pow(0.25*second, n);
        default:
            return a*pow(alpha*second, n);
    }
}

