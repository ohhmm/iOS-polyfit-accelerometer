//
//  Gait.cpp
//  AccelerometerGraph
//
//  Created by Сергей Кривонос on 28.04.17.
//
//

#include "Gait.hpp"
#include <algorithm>
#include <sstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/assert.hpp>
#include <boost/numeric/ublas/blas.hpp>
#include <boost/numeric/ublas/lu.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/vector.hpp>

#include <CoreFoundation/CoreFoundation.h>
#include <Accelerate/Accelerate.h>
///Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/System/Library/Frameworks/Accelerate.framework/Versions/A/Frameworks/vecLib.framework/Versions/A/Headers/BNNS/bnns.h

//#include <vecLib/BNNS/bnns.h>

std::string __gait_type_name[] = {
    "unknown",
    "still",
    "graze",
    "walk",
    "trot",
    "cantor",
    "galoop"
};

std::string* gait_type_name = __gait_type_name-unknown;

extern int printf(const char * __restrict format, ...);

namespace {
    
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
}


namespace boost {
    namespace serialization {
        
        template<uint N>
        struct Serialize
        {
            template<class Archive, typename... Args>
            static void serialize(Archive & ar, std::tuple<Args...> & t, const unsigned int version)
            {
                ar & std::get<N-1>(t);
                Serialize<N-1>::serialize(ar, t, version);
            }
        };
        
        template<>
        struct Serialize<0>
        {
            template<class Archive, typename... Args>
            static void serialize(Archive & ar, std::tuple<Args...> & t, const unsigned int version)
            {
            }
        };
        
        template<class Archive, typename... Args>
        void serialize(Archive & ar, std::tuple<Args...> & t, const unsigned int version)
        {
            Serialize<sizeof...(Args)>::serialize(ar, t, version);
        }
        
    }
}

std::vector<double> Gait::TimeRange = generateTimeSequence();

std::vector<double> Gait::generateTimeSequence() {
    std::vector<double> time;
    for(int i=0; i<accelerationPointsCount; ++i){
        time.push_back(i);
    }
    return time;
}

GaitType Gait::detectGait() {
    if (xx.size() == accelerationPointsCount) {
    //    constexpr int degree = 31;
    //pfitx = polyfit( TimeRange, xx, degree );
    //pfity = polyfit( TimeRange, yy, degree );
    //pfitz = polyfit( TimeRange, zz, degree );
    }
    return GaitType::unknown;
}

bool Gait::setGait(GaitType gaitType){
    this->gaitType = gaitType;

    using namespace std;
    using boost::archive::text_oarchive;
    using boost::property_tree::ptree;
    using boost::property_tree::read_json;
    using boost::property_tree::write_json;

    ptree pt;
    pt.put("Gait", gait_type_name[gaitType]);
    {
        ptree pointsJson;
        for(auto p : points) {
            ptree item;
            item.put("X", std::get<0>(p));
            item.put("Y", std::get<1>(p));
            item.put("Z", std::get<2>(p));
            pointsJson.push_back(std::make_pair("", item));
        }
        
        pt.add_child("Vectors", pointsJson);
    }
//    pt.put("PolyFitX", pfitx);
//    pt.put("PolyFitY", pfity);
//    pt.put("PolyFitZ", pfitz);

    {
    std::ostringstream s;
    write_json (s, pt, false);
    this->learningDataJson = s.str();
    }
    return detectGait() != gaitType;
}

std::string Gait::getLearningData() const {

    return learningDataJson;
}

void Gait::addAcceleration(double x, double y, double z) {
    points.push_back(point(x,y,z));
    xx.push_back(x);
    yy.push_back(y);
    zz.push_back(z);
    if(points.size() > accelerationPointsCount) {
        points.pop_front();
        xx.pop_front();
        yy.pop_front();
        zz.pop_front();
    }
    
    //printf("hi");
}

