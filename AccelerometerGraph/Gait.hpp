//
//  Gait.hpp
//  AccelerometerGraph
//
//  Created by Сергей Кривонос on 28.04.17.
//
//

#pragma once

//#include <stdio.h>
#include <deque>
#include <tuple>

enum GaitType {
    unknown = -1,
    still,
    graze,
    walk,
    trot,
    cantor,
    galoop
};


class Gait {
    static constexpr int accelerationPointsCount = 50; // five seconds
    using point = std::tuple<double,double,double>;
    std::deque<point> points;
    std::deque<double> xx;
    std::deque<double> yy;
    std::deque<double> zz;
    GaitType gaitType = still;

public:
    
    
    /**
     @returns true if training needed
     */
    bool setGait(GaitType gaitType);
    GaitType detectGait();
    void addAcceleration(double x, double y, double z);
};
