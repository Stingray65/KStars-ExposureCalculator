#ifndef IMAGINGSENSORDATA_H
#define IMAGINGSENSORDATA_H

#include <QAbstractItemModel>
#include <QVector>
#include "sensorgainreadnoise.h"

QT_BEGIN_NAMESPACE
namespace OptimalExposure
{

    typedef enum { SENSORTYPE_MONOCHROME, SENSORTYPE_COLOR } SensorType;
    typedef enum { GAIN_SELECTION_TYPE_NORMAL, GAIN_SELECTION_TYPE_ISO_DISCRETE, GAIN_SELECTION_TYPE_FIXED } GainSelectionType; // GAIN_SELECTION_TYPE_FIXED is for cameras in which read-noise does not vary with gain.


    class ImagingSensorData
    {

    public:
        ImagingSensorData();
        ImagingSensorData(const QString &sensorId, OptimalExposure::SensorType sensorType, OptimalExposure::GainSelectionType gainSelectionType, const QVector<int> &gainSelectionRange, const QVector<OptimalExposure::SensorGainReadNoise> &sensorGainReadNoiseVector);

        QString getSensorId();
        void setSensorId(const QString newSensorId);

        SensorType getSensorType() const;
        void setSensorType(SensorType newSensorType);

        OptimalExposure::GainSelectionType getGainSelectionType() const;
        void setGainSelectionType(OptimalExposure::GainSelectionType newGainSelectionType);

        int getGainMin();
        int getGainMax();

        QVector<SensorGainReadNoise> getSensorGainReadNoiseVector();
        void setSensorGainReadNoiseVector(QVector<SensorGainReadNoise> newSensorGainReadNoiseVector);

        QVector<int> getGainSelectionRange();
        void setGainSelectionRange(QVector<int> newGainSelectionRange);

    private:
        QString sensorId;
        OptimalExposure::SensorType sensorType;
        OptimalExposure::GainSelectionType gainSelectionType;

        // For GAIN_SELECTION_TYPE_NORMAL gainSelection holds only the min and max gains.
        // For GAIN_SELECTION_TYPE_ISO_DISCRETE, gainSelection hold the discrete values.
        // For GAIN_SELECTION_TYPE_FIXED the gainSelection will not be populated
        QVector<int> gainSelectionRange;
        QVector<OptimalExposure::SensorGainReadNoise> sensorGainReadNoiseVector;

    };
}
QT_END_NAMESPACE
#endif // IMAGINGSENSORDATA_H
