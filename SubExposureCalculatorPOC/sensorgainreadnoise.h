#ifndef SENSORGAINREADNOISE_H
#define SENSORGAINREADNOISE_H

#include <QAbstractItemModel>
QT_BEGIN_NAMESPACE
namespace OptimalExposure {

    class SensorGainReadNoise
    {

    public:
        SensorGainReadNoise();
        SensorGainReadNoise(int gain, double readNoise);

        int getGain();
        void setGain(int newGain);
        double getReadNoise();
        void setReadNoise(double newReadNoise);

    private:
        int gain = 0;
        double readNoise = 0.0;

    };


}
QT_END_NAMESPACE
#endif // SENSORGAINREADNOISE_H



