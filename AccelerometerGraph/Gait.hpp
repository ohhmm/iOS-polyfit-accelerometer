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
#include <string>
#include <tuple>
#include <vector>

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
    std::deque<double> xx, yy, zz;
    GaitType gaitType = still;
    std::vector<double> pfitx, pfity, pfitz;
    static std::vector<double> TimeRange;
    static std::vector<double> generateTimeSequence();
    std::string learningDataJson;

public:
    
    /**
     @returns true if training needed
     */
    bool setGait(GaitType gaitType);
    std::string getLearningData() const;
    GaitType detectGait();
    void addAcceleration(double x, double y, double z);
};
