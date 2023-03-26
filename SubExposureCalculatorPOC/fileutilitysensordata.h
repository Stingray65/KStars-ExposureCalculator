#include <QAbstractItemModel>
#include "imagingsensordata.h"
#include "sensorgainreadnoise.h"

#ifndef FILEUTILITYSENSORDATA_H
#define FILEUTILITYSENSORDATA_H


QT_BEGIN_NAMESPACE
namespace OptimalExposure
{
    class FileUtilitySensorData
    {

    public:
        int static readSensorDataFile(QString sensorId, ImagingSensorData *anImagingSensorData);
        int static writeSensorDataFile(ImagingSensorData *anImagingSensorData);
        void static buildSensorDataFile();
    };
}

QT_END_NAMESPACE
#endif // FILEUTILITYSENSORDATA_H
