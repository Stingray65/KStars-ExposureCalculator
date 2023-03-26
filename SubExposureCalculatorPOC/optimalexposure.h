#ifndef OPTIMALEXPOSURE_H
#define OPTIMALEXPOSURE_H
#include <QAbstractItemModel>
#include <QVector>
#include "optimalexposurestack.h"

QT_BEGIN_NAMESPACE
namespace OptimalExposure
{
    class OptimalExposureDetail
    {
        public:
            OptimalExposureDetail();

        private:
            int SubExposureTime;
            double exposurePollutionElectrons;
            double exposureShotNoise;
            double exposureTotalNoise;
            QVector<OptimalExposureStack> stackSummary;
    };
}
QT_END_NAMESPACE

#endif // OPTIMALEXPOSURE_H
