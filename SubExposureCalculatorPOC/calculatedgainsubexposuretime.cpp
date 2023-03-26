#include <QDebug>
#include "calculatedgainsubexposuretime.h"
#include <QAbstractItemModel>

// int subExposureGain = 0;
// double subExposureTime = 0.0;



int OptimalExposure::CalculatedGainSubExposureTime::getSubExposureGain()
{
    return subExposureGain;
}

void OptimalExposure::CalculatedGainSubExposureTime::setSubExposureGain(int newSubExposureGain)
{
    subExposureGain = newSubExposureGain;
}

double OptimalExposure::CalculatedGainSubExposureTime::getSubExposureTime()
{
    return subExposureTime;
}

void OptimalExposure::CalculatedGainSubExposureTime::setSubExposureTime(double newSubExposureTime)
{
    subExposureTime = newSubExposureTime;
}



namespace OptimalExposure {
    CalculatedGainSubExposureTime::CalculatedGainSubExposureTime(int subExposureGain, double subExposureTime) : subExposureGain(subExposureGain), subExposureTime(subExposureTime)
    {
         qDebug() << "CalculatedGainSubExposureTime constructor: " << subExposureGain << " " << subExposureTime;
    }

}
