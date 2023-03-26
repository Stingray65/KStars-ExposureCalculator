#include <QDebug>
#include <cmath>
#include "optimalsubexposurecalculator.h"
#include "imagingsensordata.h"
#include "calculatedgainsubexposuretime.h"
#include "cameraexposureenvelope.h"
#include "optimalexposuredetail.h"

namespace OptimalExposure {

/*
 *  For UI presentation provide two calculation methods.
 *  1) A method for the overall calculation for
 *  exposure times over the range of gain read-noise pairs.
 *      This method would be called when changes are made to:
 *          Camera selection, Focal Ratio, Sky Quality, Filter Compensation, or Noise Tolerance
 *      This method will return an obect (CameraExposureEnvelope) containing (for ui presentation)
 *          lightPollutionElectronBaseRate, lightPollutionForOpticFocalRatio,
 *          a vector of gain sub-exposure pairs, and int values the max and min exposure time.
 *      This method is intented to be used to present the graphical display of exposures for the camera,
 *      based on a specifc SQM.  The SQM can be adjusted in the UI, and an this method will be used to refresh ui.
 *
 *  2) A method for a calculation of an exposure time (OptimalExposureDetail) at a specific gain.
 *      This method is intented to be used to present a specific exposure calculation, and
 *      resulting shot and stack noise information.  This method will interpolate for a
 *      read-noise value (between sensor gain read-noise pairs), and will be called when
 *      a change is made to selected Gain on the UI.
 *      This method will return an object that conains:
 *          Precise Exposure Time in (interpolated), Shot pollution electrons, Exposure shot noise, Exposure total noise
 *          a Vector for stacks of 1 to n hours, which includes Exposure Count, Stack Total Time, Stack total noise
 *
 */

/*
 *
 * More work is needed for the calculation of effect of a Filter on exposure noise and time.
 * Original effort (in Java) used an estimage of the spectrum bandwidth passed by a filter. For
 * example: on broadband filters for a one shot color camera:
 *      Optolong l-Pro apprears to pass about 165 nanometers.
 *      Optolong l-Enhance apprears to pass only about 33 nanometers.
 *
 * In this code the filter compensation has is applied as a reducer of the calulation of the
 * light pollution rate for the optic. For example, the filter compensation to the light pollution
 * would be 165 / 300 for an l-Pro filter.
 *
 * But this filter compensatoin approach is imprecise & and does not reflect reality. It might
 * be better to analyze a filter for its ability to block the distinct emmission lines found
 * in light pollution:
 *
 *      Hg	435.8, 546.1, 577, 578.1
 *      Na	598, 589.6, 615.4, 616.1
 *
 * And finally, the use of 300 nanometers (the bandwidth of the visible light spectrum) as the
 * basis for filter compensation may not be accurate. An unfiltered camera sensor may be able
 * to "see" a spectrum that is much more broad.  Is an unfiltered sensor collecting more noise
 * from beyond the range of the visible spectrum?  But other discussions on web forums regarding
 * Dr. Glovers calculations appear to use 300 as the basis for filter compensation.
 *
 */

OptimalSubExposureCalculator::OptimalSubExposureCalculator(
    double aNoiseTolerance, double aSkyQuality, double aFocalRatio, double aFilterCompensation, ImagingSensorData &anImagingSensorData) :
    aNoiseTolerance(aNoiseTolerance),
    aSkyQuality(aSkyQuality),
    aFocalRatio(aFocalRatio),
    aFilterCompensation(aFilterCompensation),
    anImagingSensorData(anImagingSensorData) {}



    double OptimalSubExposureCalculator::getASkyQuality()
    {
        return aSkyQuality;
    }

    void OptimalSubExposureCalculator::setASkyQuality(double newASkyQuality)
    {
        aSkyQuality = newASkyQuality;
    }

    double OptimalSubExposureCalculator::getANoiseTolerance()
    {
        return aNoiseTolerance;
    }

    void OptimalSubExposureCalculator::setANoiseTolerance(double newANoiseTolerance)
    {
        aNoiseTolerance = newANoiseTolerance;
    }

    double OptimalSubExposureCalculator::getAFocalRatio()
    {
        return aFocalRatio;
    }

    void OptimalSubExposureCalculator::setAFocalRatio(double newAFocalRatio)
    {
        aFocalRatio = newAFocalRatio;
    }

    double OptimalSubExposureCalculator::getAFilterCompensation()
    {
        return aFilterCompensation;
    }

    void OptimalSubExposureCalculator::setAFilterCompensation(double newAFilterCompensation)
    {
        aFilterCompensation = newAFilterCompensation;
    }

    ImagingSensorData &OptimalSubExposureCalculator::getImagingSensorData()
    {
        return anImagingSensorData;
    }

    void OptimalSubExposureCalculator::setImagingSensorData(ImagingSensorData &newanImagingSensorData)
    {
        anImagingSensorData = newanImagingSensorData;
    }

    int OptimalSubExposureCalculator::getASelectedGain()
    {
        return aSelectedGain;
    }

    void OptimalSubExposureCalculator::setASelectedGain(int newASelectedGain)
    {
        aSelectedGain = newASelectedGain;
    }



    QVector<CalculatedGainSubExposureTime> OptimalSubExposureCalculator::calculateGainSubExposureVector(double cFactor, double lightPollutionForOpticFocalRatio){
        qDebug() << "Calculating gain sub-exposure vector: ";
        QVector<CalculatedGainSubExposureTime> aCalculatedGainSubExposureTimeVector;

        QVector<OptimalExposure::SensorGainReadNoise> aSensorGainReadNoiseVector = anImagingSensorData.getSensorGainReadNoiseVector();

        for(QVector<OptimalExposure::SensorGainReadNoise>::iterator rn = aSensorGainReadNoiseVector.begin(); rn != aSensorGainReadNoiseVector.end(); ++rn){
            int aGain = rn->getGain();
            qDebug() << "At a gain of: " << aGain;
            double aReadNoise = rn->getReadNoise();
            qDebug() << "\t sensor read-noise is: " << aReadNoise;

            // initial sub exposure.
            double anOptimalSubExposure = 0.0;


            switch(anImagingSensorData.getSensorType()) {
            case SENSORTYPE_MONOCHROME:
                anOptimalSubExposure =  cFactor * pow(aReadNoise,2) / lightPollutionForOpticFocalRatio;
                break;

            case SENSORTYPE_COLOR:
                anOptimalSubExposure = (double)3.0 * cFactor * pow(aReadNoise,2) / lightPollutionForOpticFocalRatio;
                break;
            }

            /*  Need to improve the application of filter compensation before implementing
             *  But maybe the CalculatedGainSubExposureTime should be changed to carry both an unfiltered and filetered exposure value
             *  Java code was using a simple multiplier of the exposure time to compensate for the effect of a filter.
             *  But maybe filter compensation should be applied to the sky quality and alter the light pollution rate

                double filterMultiplier =  (double)FilterBandpass.STANDARD_BANDPASS_WIDTH / (double)aFilter.getBandPassNanometers();
                double optimalSubExposureFiltered = optimalSubExposure * filterMultiplier;
            */

            qDebug() << "anOptimalSubExposure is: " << anOptimalSubExposure;

            CalculatedGainSubExposureTime *aSubExposureTime = new CalculatedGainSubExposureTime(aGain, anOptimalSubExposure);

            aCalculatedGainSubExposureTimeVector.append(*aSubExposureTime);

        }

        return aCalculatedGainSubExposureTimeVector;
    }
/*
    double OptimalSubExposureCalculator::calculateLightPolutionForOpticFocalRatio(double lightPollutionElectronBaseRate, double aFocalRatio) {
    // double lightPollutionRateForOptic =  lightPollutionElectronBaseRate * java.lang.Math.pow(anOptic.getFocalRatio(),-2);
        return((double) (lightPollutionElectronBaseRate * pow(aFocalRatio,-2)));
    }
*/

    double OptimalSubExposureCalculator::calculateLightPolutionForOpticFocalRatio(double lightPollutionElectronBaseRate, double aFocalRatio, double aFilterCompensation) {
    // double lightPollutionRateForOptic =  lightPollutionElectronBaseRate * java.lang.Math.pow(anOptic.getFocalRatio(),-2);
        return(((double) (lightPollutionElectronBaseRate * pow(aFocalRatio,-2))) * aFilterCompensation);
    }

    double OptimalSubExposureCalculator::calculateCFactor(double aNoiseTolerance) {
        // cFactor = 1/( (((100+allowableNoiseIncreasePercent)/100)^2) -1)
        return((double) (1/( pow((((double)100 + (double) aNoiseTolerance)/(double)100),(double)2) -1)));

    }

    double OptimalSubExposureCalculator::calculateLightPollutionElectronBaseRate(double skyQuality) {
        // Conversion curve fitted from Dr Glover data
        double base = std::stod("1.25286030612621E+27");
        double power = (double)-19.3234809465887;
        return(base * pow(skyQuality, power));
    }

    OptimalExposure::CameraExposureEnvelope OptimalSubExposureCalculator::calculateCameraExposureEnvelope(){
        /*
            This method calculates the exposures for each gain setting found in the sensor gain readnoise table.
            It is used to refresh the ui presentation, in prparation for a calculation of sub-exposure data with a
            specific (probably interpolated) read-noise value.
        */

        qDebug() << "Calculating CameraExposureEnvelope...";

        qDebug() << "Using Sky Quality: " << aSkyQuality;
        double lightPollutionElectronBaseRate = OptimalSubExposureCalculator::calculateLightPollutionElectronBaseRate(aSkyQuality);
        qDebug() << "\tConverted to lightPollutionElectronBaseRate: " << lightPollutionElectronBaseRate;

        qDebug() << "Using an Optical Focal Ratio: " << aFocalRatio;

        double lightPollutionForOpticFocalRatio = calculateLightPolutionForOpticFocalRatio(lightPollutionElectronBaseRate, aFocalRatio, aFilterCompensation);
        qDebug() << "\tResulting in an Light Pollution Rate for the Optic of: " << lightPollutionForOpticFocalRatio;

        qDebug() << "Using a camera Id: " << anImagingSensorData.getSensorId();
        qDebug() << "\tWith a read-noise table of : " << anImagingSensorData.getSensorGainReadNoiseVector().size() << " values";

        qDebug() << "Using a Noise Tolerance of: " << aNoiseTolerance;

        double cFactor = calculateCFactor(aNoiseTolerance);
        qDebug() << "Calculated CFactor is: " << cFactor;

        QVector<CalculatedGainSubExposureTime> aSubExposureTimeVector = calculateGainSubExposureVector(cFactor, lightPollutionForOpticFocalRatio);

        double exposureTimeMin = 99999999999.9;
        double exposureTimeMax = -1.0;
        // Iterate the times to capture and store the min and max exposure times.
        // (But the QCustomPlot can rescale, so this may be unnecessary
        for(QVector<OptimalExposure::CalculatedGainSubExposureTime>::iterator oe = aSubExposureTimeVector.begin(); oe != aSubExposureTimeVector.end(); ++oe){
            if(exposureTimeMin > oe->getSubExposureTime()) exposureTimeMin = oe->getSubExposureTime();
            if(exposureTimeMax < oe->getSubExposureTime()) exposureTimeMax = oe->getSubExposureTime();
        }

        CameraExposureEnvelope aCameraExposureEnvelope = CameraExposureEnvelope(
            lightPollutionElectronBaseRate,
            lightPollutionForOpticFocalRatio,
            aSubExposureTimeVector,
            exposureTimeMin,
            exposureTimeMax);

        return(aCameraExposureEnvelope);
    }

    OptimalExposure::OptimalExposureDetail OptimalSubExposureCalculator::calculateSubExposureDetail(int aSelectedGainValue){
        /*
            This method calculates some details for a sub-exposure. It will interpolate between
            points in the sensor read-noise table, to find a reasonable appoximation of the read-noise
            for a non-listed gain vale.
        */

        qDebug() << "aSelectedGainValue (from ui gainSlider adjustment): " << aSelectedGainValue;


        // Look for a matching gain from the sensor gain read-noise table, or identify a bracket for interpolation.
        // Interpolation may result in slight errors when the read-noise data is curve. (there's probably a better way to code this)
        double aReadNoise = 100.0;  // defaulted high just to make an error in the interpolation obvious
        bool matched = false;
        int lowerReadNoiseIndex = 0;
        QVector<OptimalExposure::SensorGainReadNoise> aSensorGainReadNoiseVector = anImagingSensorData.getSensorGainReadNoiseVector();
        for(int readNoiseIndex=0; readNoiseIndex<anImagingSensorData.getSensorGainReadNoiseVector().size(); readNoiseIndex++){
            if(!matched){
                SensorGainReadNoise aSensorGainReadNoise = anImagingSensorData.getSensorGainReadNoiseVector()[readNoiseIndex];
                if(aSensorGainReadNoise.getGain() == aSelectedGainValue){
                    matched = true;
                    qDebug() << " matched a sensor gain ";
                    aReadNoise = anImagingSensorData.getSensorGainReadNoiseVector()[readNoiseIndex].getReadNoise();
                    break;
                }
                if(aSensorGainReadNoise.getGain() < aSelectedGainValue){
                    lowerReadNoiseIndex = readNoiseIndex;
                }
            }
        }

        if(!matched){
            qDebug() << "Interpolating read noise gain bracket: " << lowerReadNoiseIndex << " and " << lowerReadNoiseIndex+1;
            if (lowerReadNoiseIndex < anImagingSensorData.getSensorGainReadNoiseVector().size()-1){
                SensorGainReadNoise aLowerIndexSensorReadNoise = anImagingSensorData.getSensorGainReadNoiseVector()[lowerReadNoiseIndex];
                SensorGainReadNoise anUpperIndexSensorReadNoise = anImagingSensorData.getSensorGainReadNoiseVector()[lowerReadNoiseIndex+1];

                // interpolate a read-noise value
                double m = (anUpperIndexSensorReadNoise.getReadNoise() - aLowerIndexSensorReadNoise.getReadNoise()) / (anUpperIndexSensorReadNoise.getGain() - aLowerIndexSensorReadNoise.getGain());
                aReadNoise = aLowerIndexSensorReadNoise.getReadNoise() + (m * (aSelectedGainValue - aLowerIndexSensorReadNoise.getGain()));
                qDebug() << "The sensor read-noise for the selected gain value is interpolated to: " << aReadNoise;
            }else{
                qDebug() << " using max sensor gain ";
                aReadNoise = anImagingSensorData.getSensorGainReadNoiseVector()[anImagingSensorData.getSensorGainReadNoiseVector().size()-1].getReadNoise();
            }
        }else{
            qDebug() << "The sensor read-noise for the selected gain value was matched: " << aReadNoise;
        }


        double anOptimalSubExposure = 0.0;

        double lightPollutionElectronBaseRate = OptimalSubExposureCalculator::calculateLightPollutionElectronBaseRate(aSkyQuality);
        double lightPollutionForOpticFocalRatio = calculateLightPolutionForOpticFocalRatio(lightPollutionElectronBaseRate, aFocalRatio, aFilterCompensation);
        double cFactor = calculateCFactor(aNoiseTolerance);

        switch(anImagingSensorData.getSensorType()) {
        case SENSORTYPE_MONOCHROME:
            anOptimalSubExposure =  cFactor * pow(aReadNoise,2) / lightPollutionForOpticFocalRatio;
            break;

        case SENSORTYPE_COLOR:
            anOptimalSubExposure = (double)3.0 * cFactor * pow(aReadNoise,2) / lightPollutionForOpticFocalRatio;
            break;
        }

        qDebug() << "an Optimal Sub Exposure at gain: " << aSelectedGainValue << " is " << anOptimalSubExposure << " seconds";
        // Add the single exposure noise calcs to the response
        double anExposurePollutionElectrons = lightPollutionForOpticFocalRatio * anOptimalSubExposure;
        qDebug() << "Exposure Pollution Electrons: " << anExposurePollutionElectrons;

        double anExposureShotNoise = sqrt(lightPollutionForOpticFocalRatio * anOptimalSubExposure);
        qDebug() << "Exposure Shot Noise: " << anExposureShotNoise;

        double anExposureTotalNoise = sqrt( pow(aReadNoise, 2)  + lightPollutionForOpticFocalRatio * anOptimalSubExposure);
        qDebug() << "Exposure Total Noise: " << anExposureTotalNoise;

        // Need to build and populate the stack estimations
        QVector<OptimalExposureStack> aStackSummary;

        // Loop through sessions of 1 to 5 hours. (Maybe a preferences selection for this?)
        for(int sessionHours = 1; sessionHours < 6; sessionHours++){
            int anExposureCount = (int) ceil( (double)(sessionHours * 3600) / anOptimalSubExposure ); // rounding up to ensure that exposure count is at least "sessionHours" of data.
            int aStackTime = anExposureCount * anOptimalSubExposure;
            double aStackTotalNoise = sqrt( (double)anExposureCount * pow(aReadNoise,2) + lightPollutionForOpticFocalRatio * (anExposureCount * anOptimalSubExposure));

            OptimalExposureStack *anOptimalExposureStack = new OptimalExposureStack(sessionHours, anExposureCount, aStackTime, aStackTotalNoise);
            aStackSummary.push_back(*anOptimalExposureStack);
        }

        OptimalExposureDetail anOptimalExposureDetail = OptimalExposureDetail(aSelectedGainValue, anOptimalSubExposure, anExposurePollutionElectrons, anExposureShotNoise,  anExposureTotalNoise, aStackSummary);

        return(anOptimalExposureDetail);
    }


}

