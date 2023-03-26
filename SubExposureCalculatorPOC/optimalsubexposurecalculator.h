#ifndef OPTIMALSUBEXPOSURECALCULATOR_H
#define OPTIMALSUBEXPOSURECALCULATOR_H
#include <QAbstractItemModel>
#include <QVector>
#include "imagingsensordata.h"
#include "calculatedgainsubexposuretime.h"
#include "cameraexposureenvelope.h"
#include "optimalexposuredetail.h"

QT_BEGIN_NAMESPACE
namespace OptimalExposure {


    class OptimalSubExposureCalculator
    {
        public:
            OptimalSubExposureCalculator();
            OptimalSubExposureCalculator(double aNoiseTolerance, double aSkyQuality, double aFocalRatio, double aFilterCompensation, ImagingSensorData &aCalculationImagingSensorData);

            CameraExposureEnvelope calculateCameraExposureEnvelope();
            OptimalExposureDetail calculateSubExposureDetail(int aSelectedGainValue);

            double getANoiseTolerance();
            void setANoiseTolerance(double newNoiseTolerance);

            double getASkyQuality();
            void setASkyQuality(double newSkyQuality);

            double getAFocalRatio();
            void setAFocalRatio(double newFocalRatio);

            double getAFilterCompensation();
            void setAFilterCompensation(double newFilterCompensation);

            ImagingSensorData &getImagingSensorData();
            void setImagingSensorData(ImagingSensorData &newACalculationImagingSensorData);


            int getASelectedGain();
            void setASelectedGain(int newSelectedGain);

    protected:
            double aNoiseTolerance;
            double aSkyQuality;
            double aFocalRatio;
            double aFilterCompensation;
            int aSelectedGain;
            ImagingSensorData anImagingSensorData;


        private:
            double calculateCFactor(double noiseTolerance);
            double calculateLightPollutionElectronBaseRate(double skyQuality);
            double calculateLightPolutionForOpticFocalRatio(double lightPollutionElectronBaseRate, double aFocalRatio, double AFilterCompensation);
            QVector<CalculatedGainSubExposureTime> calculateGainSubExposureVector(double cFactor, double lightPollutionForOpticFocalRatio);


    };

}
QT_END_NAMESPACE

#endif // OPTIMALSUBEXPOSURECALCULATOR_H
