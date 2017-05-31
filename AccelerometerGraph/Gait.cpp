//
//  Gait.cpp
//  AccelerometerGraph
//
//  Created by Сергей Кривонос on 28.04.17.
//
//

#include "Gait.hpp"
#include <algorithm>
#include <boost/assert.hpp>
#include <boost/numeric/ublas/blas.hpp>
#include <boost/numeric/ublas/lu.hpp>
#include <CoreFoundation/CoreFoundation.h>
#include <Accelerate/Accelerate.h>
///Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/System/Library/Frameworks/Accelerate.framework/Versions/A/Frameworks/vecLib.framework/Versions/A/Headers/BNNS/bnns.h

//#include <vecLib/BNNS/bnns.h>


extern int printf(const char * __restrict format, ...);

namespace {
    struct UniqueNumber {
        double current = 0;
        double operator()() {return current++;}
    };
    
    std::vector<double> generateTimeSequence() {
        std::vector<double> time;
        std::generate (time.begin(), time.end(), UniqueNumber());
        return time;
    }
    
    std::vector<double> TimeRange = generateTimeSequence();
}


template <class T, template<class> class CX, template<class> class CY>
std::vector<T> polyfit1( const CX<T>& oX, const CY<T>& oY, int nDegree )
{
    using namespace boost::numeric::ublas;
    
    size_t nCount =  oX.size() / nDegree;
    if ( nCount != oY.size() )
        throw std::invalid_argument( "X and Y vector sizes do not match" );
    
    // more intuative this way
    nDegree++;
    
    matrix<T> oXMatrix( nCount, nDegree );
    matrix<T> oYMatrix( nCount, 1 );
    
    // copy y matrix
    for ( size_t i = 0; i < nCount; i++ )
    {
        oYMatrix(i, 0) = oY[i];
    }
    
    // create the X matrix
    for ( size_t nRow = 0; nRow < nCount; nRow++ )
    {
        T nVal = 1.0f;
        for ( int nCol = 0; nCol < nDegree; nCol++ )
        {
            oXMatrix(nRow, nCol) = nVal;
            nVal *= oX[nRow];
        }
    }
    
    // transpose X matrix
    matrix<T> oXtMatrix( trans(oXMatrix) );
    // multiply transposed X matrix with X matrix
    matrix<T> oXtXMatrix( prec_prod(oXtMatrix, oXMatrix) );
    // multiply transposed X matrix with Y matrix
    matrix<T> oXtYMatrix( prec_prod(oXtMatrix, oYMatrix) );
    
    // lu decomposition
    permutation_matrix<int> pert(oXtXMatrix.size1());
    const std::size_t singular = lu_factorize(oXtXMatrix, pert);
    // must be singular
    BOOST_ASSERT( singular );
    
    // backsubstitution
    lu_substitute(oXtXMatrix, pert, oXtYMatrix);
    
    // copy the result to coeff
    return std::vector<T>( oXtYMatrix.data().begin(), oXtYMatrix.data().end() );
}

template<class T>
std::vector<T> polyfit( const std::vector<T>& oX,
                       const std::deque<T>& oY, int nDegree )
{
    using namespace boost::numeric::ublas;
    
    if ( oX.size() != oY.size() )
        throw std::invalid_argument( "X and Y vector sizes do not match" );
    
    // more intuative this way
    nDegree++;
    
    size_t nCount =  oX.size();
    matrix<T> oXMatrix( nCount, nDegree );
    matrix<T> oYMatrix( nCount, 1 );
    
    // copy y matrix
    for ( size_t i = 0; i < nCount; i++ )
    {
        oYMatrix(i, 0) = oY[i];
    }
    
    // create the X matrix
    for ( size_t nRow = 0; nRow < nCount; nRow++ )
    {
        T nVal = 1.0f;
        for ( int nCol = 0; nCol < nDegree; nCol++ )
        {
            oXMatrix(nRow, nCol) = nVal;
            nVal *= oX[nRow];
        }
    }
    
    // transpose X matrix
    matrix<T> oXtMatrix( trans(oXMatrix) );
    // multiply transposed X matrix with X matrix
    matrix<T> oXtXMatrix( prec_prod(oXtMatrix, oXMatrix) );
    // multiply transposed X matrix with Y matrix
    matrix<T> oXtYMatrix( prec_prod(oXtMatrix, oYMatrix) );
    
    // lu decomposition
    permutation_matrix<int> pert(oXtXMatrix.size1());
    const std::size_t singular = lu_factorize(oXtXMatrix, pert);
    // must be singular
    BOOST_ASSERT( singular == 0 );
    
    // backsubstitution
    lu_substitute(oXtXMatrix, pert, oXtYMatrix);
    
    // copy the result to coeff
    return std::vector<T>( oXtYMatrix.data().begin(), oXtYMatrix.data().end() );
}

template<typename T>
std::vector<T> polyval( const std::vector<T>& oCoeff,
                       const std::vector<T>& oX )
{
    size_t nCount =  oX.size();
    size_t nDegree = oCoeff.size();
    std::vector<T>	oY( nCount );
    
    for ( size_t i = 0; i < nCount; i++ )
    {
        T nY = 0;
        T nXT = 1;
        T nX = oX[i];
        for ( size_t j = 0; j < nDegree; j++ )
        {
            // multiply current x by a coefficient
            nY += oCoeff[j] * nXT;
            // power up the X
            nXT *= nX;
        }
        oY[i] = nY;
    }
    
    return oY;
}

GaitType Gait::detectGait() {
    auto pfitx = polyfit( TimeRange, xx, accelerationPointsCount );
    auto pfity = polyfit( TimeRange, yy, accelerationPointsCount );
    auto pfitz = polyfit( TimeRange, zz, accelerationPointsCount );
    return GaitType::unknown;
}

bool Gait::setGait(GaitType gaitType){
    this->gaitType = gaitType;
    return detectGait() == gaitType;
}


void Gait::addAcceleration(double x, double y, double z) {
    points.push_back(point(x,y,z));
    xx.push_back(x);
    yy.push_back(y);
    zz.push_back(z);
    if(points.size()>accelerationPointsCount) {
        points.pop_front();
        xx.pop_front();
        yy.pop_front();
        zz.pop_front();
    }
    
    printf("hi");
}

