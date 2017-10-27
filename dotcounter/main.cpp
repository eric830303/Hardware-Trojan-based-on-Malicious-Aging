//
//  main.cpp
//  RegionClassification
//
//  Created by TienHungTseng on 2017/7/3.
//  Copyright © 2017年 Eric Tseng. All rights reserved.
//

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

using namespace std ;


int main(int argc, const char * argv[])
{
    if( argc < 4 )
    {
        printf("./counter file.txt year(int) error(double) \n") ;
        return -1 ;
    }
    
    ifstream        file  ;
    string          line  ;
    
    file.open( argv[1], ios::in ) ;
    
    if( !file )
    {
        printf("Can't Read file\n") ;
    }
    double year  = atof( argv[2] ) ;
    double error = atof( argv[3] ) ;
    
    int R1 = 0 ;// L < n-e ; R < n-e
    int R2 = 0 ;// L < n-e ; n-e < R < n+e
    int R3 = 0 ;// L < n-e ; n+e < R
    int R4 = 0 ;// n-e < L < n+e ; n+e < R
    int R5 = 0 ;// n+e < L ; n+e < R
    int R6 = 0 ;// n-e < L < n+e ; n-e < R < n+e

    double L = year - error ;
    double R = year + error ;
    int ctr = 0 ;
    //--------------- Find PDP Info ---------------------------------------------
    while( getline( file, line ) )
    {
        double x = 0 ;
        double y = 0 ;
        ctr++ ;
        
        istringstream   token( line )     ;
        token >> x >> y ;
        //printf("%f %f \n", x, y );
        
        if( x < L )
        {
            if( y < L )        R1++ ;
            else if( y < R )   R2++ ;
            else               R3++ ;
        }
        else if( x < R )
        {
            if(  y < R )       R6++ ;
            else               R4++ ;
        }
        else                   R5++ ;
        
    }
    printf( "R1 = %d, R2 = %d, R3 = %d, R4 = %d, R5 = %d, R6 = %d\n", R1, R2, R3, R4, R5, R6) ;
    printf( "ctr = %d\n", ctr ) ;
}
