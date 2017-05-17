//
//  Gait.hpp
//  AccelerometerGraph
//
//  Created by Сергей Кривонос on 28.04.17.
//
//

#pragma once

#include <stdio.h>
#include <deque>
#include <tuple>

enum GaitType {
    still, graze, walk, trot, cantor, galoop
};


class Gait {
    int accelerationPointsCount = 50; // five seconds
    using point = std::tuple<double,double,double>;
    std::deque<point> points;
    GaitType gaitType = still;

public:
    void setGait(GaitType gaitType){
        this->gaitType = gaitType;
    }
    
    void addAcceleration(double x, double y, double z) {
        points.push_back(point(x,y,z));
        if(points.size()>accelerationPointsCount)
            points.pop_front();
        
    }
};
