// gainReadNoise::gainReadNoise()
class GainReadNoise
{
public:
    GainReadNoise(int gain, double readNoise);

    int getGain() const;
    void setGain(int newGain);

    double getReadNoise() const;
    void setReadNoise(double newReadNoise);

private:
        int gain;
        double readNoise;


};

int GainReadNoise::getGain() const
{
    return gain;
}

void GainReadNoise::setGain(int newGain)
{
    gain = newGain;
}

double GainReadNoise::getReadNoise() const
{
    return readNoise;
}

void GainReadNoise::setReadNoise(double newReadNoise)
{
    readNoise = newReadNoise;
}

GainReadNoise::GainReadNoise(int gain, double readNoise) : gain(gain), readNoise(readNoise) {}
