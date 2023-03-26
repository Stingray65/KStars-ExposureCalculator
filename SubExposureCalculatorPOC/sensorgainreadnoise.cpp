#include <QDebug>
#include "sensorgainreadnoise.h"
#include <QAbstractItemModel>

int OptimalExposure::SensorGainReadNoise::getGain()
{
    return gain;
}

void OptimalExposure::SensorGainReadNoise::setGain(int newGain)
{
    gain = newGain;
}

double OptimalExposure::SensorGainReadNoise::getReadNoise()
{
    return readNoise;
}

void OptimalExposure::SensorGainReadNoise::setReadNoise(double newReadNoise)
{
    readNoise = newReadNoise;
}


namespace OptimalExposure {
SensorGainReadNoise::SensorGainReadNoise(int gain, double readNoise) : gain(gain), readNoise(readNoise) { /* qDebug() << "SensorGainReadNoise constructor: " << gain << " " << readNoise; */ }
}
