#include "cameraexposureenvelope.h"


namespace OptimalExposure {
    double CameraExposureEnvelope::getLightPollutionElectronBaseRate() const
    {
        return lightPollutionElectronBaseRate;
    }

    double CameraExposureEnvelope::getLightPollutionForOpticFocalRatio() const
    {
        return lightPollutionForOpticFocalRatio;
    }

    const QVector<CalculatedGainSubExposureTime> &CameraExposureEnvelope::getASubExposureVector() const
    {
        return aSubExposureVector;
    }

    double CameraExposureEnvelope::getExposureTimeMin() const
    {
        return exposureTimeMin;
    }

    double CameraExposureEnvelope::getExposureTimeMax() const
    {
        return exposureTimeMax;
    }

    CameraExposureEnvelope::CameraExposureEnvelope(
        double lightPollutionElectronBaseRate,
        double lightPollutionForOpticFocalRatio,
        const QVector<CalculatedGainSubExposureTime> &aSubExposureVector,
        double exposureTimeMin,
        double exposureTimeMax) :
        lightPollutionElectronBaseRate(lightPollutionElectronBaseRate),
        lightPollutionForOpticFocalRatio(lightPollutionForOpticFocalRatio),
        aSubExposureVector(aSubExposureVector),
        exposureTimeMin(exposureTimeMin),
        exposureTimeMax(exposureTimeMax) {}

}
