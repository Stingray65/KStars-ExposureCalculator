#include "sensorgainreadnoise.h"
#include "imagingsensordata.h"
#include <QAbstractItemModel>
#include <QVector>

namespace OptimalExposure {

QString OptimalExposure::ImagingSensorData::getSensorId()
{
    return sensorId;
}

void OptimalExposure::ImagingSensorData::setSensorId(QString newSensorId)
{
    sensorId = newSensorId;
}

OptimalExposure::SensorType OptimalExposure::ImagingSensorData::getSensorType() const
{
    return sensorType;
}

void OptimalExposure::ImagingSensorData::setSensorType(SensorType newSensorType)
{
    sensorType = newSensorType;
}

OptimalExposure::GainSelectionType ImagingSensorData::getGainSelectionType() const
{
    return gainSelectionType;
}

void ImagingSensorData::setGainSelectionType(OptimalExposure::GainSelectionType newGainSelectionType)
{
    gainSelectionType = newGainSelectionType;
}

int OptimalExposure::ImagingSensorData::getGainMin()
{
    int gainMin = 0;
    if(getGainSelectionRange().count()>0) gainMin = getGainSelectionRange()[0];
    return gainMin;
}

int OptimalExposure::ImagingSensorData::getGainMax()
{
    int gainMax = 0;
    if(getGainSelectionRange().count()>0) gainMax = getGainSelectionRange()[getGainSelectionRange().count()-1];
    return gainMax;
}

void ImagingSensorData::setGainSelectionRange(QVector<int> newGainSelectionRange)
{
    gainSelectionRange = newGainSelectionRange;
}

QVector<int> ImagingSensorData::getGainSelectionRange()
{
    return gainSelectionRange;
}

void OptimalExposure::ImagingSensorData::setSensorGainReadNoiseVector(QVector<SensorGainReadNoise> newSensorGainReadNoiseVector)
{
    sensorGainReadNoiseVector = newSensorGainReadNoiseVector;
}

QVector<OptimalExposure::SensorGainReadNoise> OptimalExposure::ImagingSensorData::getSensorGainReadNoiseVector()
{
    return sensorGainReadNoiseVector;
}

ImagingSensorData::ImagingSensorData(const QString &sensorId, SensorType sensorType, GainSelectionType gainSelectionType, const QVector<int> &gainSelectionRange, const QVector<SensorGainReadNoise> &sensorGainReadNoiseVector) : sensorId(sensorId),
    sensorType(sensorType),
    gainSelectionType(gainSelectionType),
    gainSelectionRange(gainSelectionRange),
//    gainMin(gainMin),
//    gainMax(gainMax),
    sensorGainReadNoiseVector(sensorGainReadNoiseVector)
{}

}
