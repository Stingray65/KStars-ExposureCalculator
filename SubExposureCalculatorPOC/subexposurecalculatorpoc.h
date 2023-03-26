#ifndef SUBEXPOSURECALCULATORPOC_H
#define SUBEXPOSURECALCULATORPOC_H

#include <QMainWindow>
#include "optimalsubexposurecalculator.h"

typedef enum { MONOCHROME, COLOR } CameraSensorType;

QT_BEGIN_NAMESPACE
namespace Ui { class SubExposureCalculatorPOC; }
QT_END_NAMESPACE

class SubExposureCalculatorPOC : public QMainWindow
{
    Q_OBJECT

    public:
        SubExposureCalculatorPOC(QWidget *parent = nullptr);
        ~SubExposureCalculatorPOC();

    public slots:
        void applyInitialInputs();  // This method is acting as a "fill-in" for initiating the calculator with data from KStars ekos/indi
        // void handleExposureEnvelopeAdjustment();  // Changes effecting the exposure envelope, (like to SQM, or Noise Tolerance)
        void handleUserAdjustment();  // Change to gain, does not change exposure envelope, but does require recalculation of shot

    public:

//        OptimalExposure::OptimalSubExposureCalculator getOptimalSubExposureCalculator();
//        void setOptimalSubExposureCalculator(OptimalExposure::OptimalSubExposureCalculator newOptimalSubExposureCalculator);

//        OptimalExposure::ImagingSensorData getImagingSensorData();
//        void setImagingSensorData(OptimalExposure::ImagingSensorData newImagingSensorData);


private:
        Ui::SubExposureCalculatorPOC *ui;

        OptimalExposure::OptimalSubExposureCalculator *anOptimalSubExposureCalculator;
        OptimalExposure::ImagingSensorData *anImagingSensorData;

        void initializeSubExposureCalculator(double aNoiseTolerance,  double aSkyQualityValue, double aFocalRatioValue, double aFilterCompensationValue, QString aSelectedImagingSensor);
        void calculateSubExposure(double aNoiseTolerance, double aSkyQualityValue, double aFocalRatioValue, double aFilterCompensationValue, int aSelectedGainValue);

        void realignGainSlider();
        int getGainSelection(OptimalExposure::GainSelectionType aGainSelectionType);



};
#endif // SUBEXPOSURECALCULATORPOC_H
