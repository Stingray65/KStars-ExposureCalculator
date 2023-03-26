#include <QDebug>
#include <QDir>
#include <QVectorIterator>
#include <QVariant>
#include <QTableWidgetItem>
#include "fileutilitysensordata.h"
#include "subexposurecalculatorpoc.h"
#include "optimalsubexposurecalculator.h"
#include "ui_subexposurecalculatorpoc.h"
#include "optimalexposuredetail.h"
#include <string>
#include <iostream>
#include <filesystem>



SubExposureCalculatorPOC::SubExposureCalculatorPOC(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SubExposureCalculatorPOC)
{
    ui->setupUi(this);

    // qDebug() << "Does ImageSensor Data folder exist: " << QDir("../ImageSensorData").exists();

    if(QDir("../ImageSensorData").exists()){
        QDir directory("../ImageSensorData");
        QStringList imageSensorFiles = directory.entryList(QStringList() << "*.xml" ,QDir::Files);
        foreach(QString filename, imageSensorFiles) {
            ui->imagingSensorSelector->addItem(filename);
        }

    }else{
        qDebug() << "Folder for ImageSensor Data '../ImageSensorData' was not found.";
    }

    ui->exposureCalculatorMockUp->setEnabled(false);

    ui->gainSelectionISODiscreteFrame->setEnabled(false);ui->gainSelectionISODiscreteFrame->setVisible(false);
    ui->gainSelectionNormalFrame->setEnabled(false); ui->gainSelectionNormalFrame->setVisible(false);
    ui->gainSelectionFixedFrame->setEnabled(false); ui->gainSelectionFixedFrame->setVisible(false);

    // initial values for the gain, will be altered when camera data is loaded
    ui->gainSlider->setMinimum(0);
    ui->gainSlider->setMaximum(100);
    ui->gainSlider->setValue(50);

    ui->indiFocalRatio->setMinimum(1.0);
    ui->indiFocalRatio->setMaximum(12.0);
    ui->indiFocalRatio->setSingleStep(0.1);
    ui->indiFocalRatio->setValue(5.0);

    ui->indiSkyQuality->setMinimum(16.00);
    ui->indiSkyQuality->setMaximum(22.00);
    ui->indiSkyQuality->setSingleStep(0.05);
    ui->indiSkyQuality->setValue(20.0);

    // Setup the "user" controls.
    ui->userSkyQuality->setMinimum(16.00);
    ui->userSkyQuality->setMaximum(22.00);
    ui->userSkyQuality->setSingleStep(0.05);
    ui->userSkyQuality->setValue(20.0);

    ui->noiseTolerance->setMinimum(0.05);
    ui->noiseTolerance->setMaximum(25.00);
    ui->noiseTolerance->setSingleStep(0.25);
    ui->noiseTolerance->setValue(5.0);

    ui->filterBandwidth->setMinimum(3);
    ui->filterBandwidth->setMaximum(300);
    ui->filterBandwidth->setValue(300);



    /*
        Experimental compensation for filters on light the pollution calculation are a bit tricky.
        Part 1...

        An unfiltered camera may include some IR and UV, and be able to read a bandwidth of say 360nm (at a reasonably high QE %).

        But for simplicity, the filter compensation calculation will be based on the range for visible light, and use a nominal
        bandwidth of 300, (roughly the bandwidth of a typical luminance filter).

        The filter compensation factor that will be applied to light pollution will be the filter bandwidth (from the UI) / 300.
        This means that a typical luminance filter would produce a "nuetral" compensation of 1.0 (300 / 300).

        But the user interface will allow selection of wider bands for true "unfiltered" exposure calculations.  Example: by selecting a
        bandwith of 360, the light pollution compensation will 1.2, calculated as (360 / 300).  This is to recognize that light pollution
        may be entering the IR and UV range of an unfiltered camera sensor.

        A Luminance filter may only pass 300nm, so the filter compensaton value would be 1.0 (300 / 300)
        An RGB filter may only pass 100nm, so the filter compensaton value would be 0.3333 = (100 / 300)
        An SHO filter may only pass 3nm, so the filter compensaton value would be 0.0100 = (3 / 300)

        Filters will reduce bandwidth, but also slightly reduce tranmission within the range that they "pass".
        The values stated are only for demonstration and testing purposes, further research is needed.

    */


    ui->qCustomPlotSubExposure->xAxis->setLabel("Gain");

    ui->qCustomPlotSubExposure->yAxis->setLabel("Exposure Time");

    ui->qCustomPlotSubExposure->addGraph();

    // For the Ekos/Indi initialzation
    connect(ui->applyInitialInput, &QPushButton::clicked, this, &SubExposureCalculatorPOC::applyInitialInputs);

    // for the user edits
    connect(ui->gainSlider, &QSlider::sliderReleased, this, &SubExposureCalculatorPOC::handleUserAdjustment);

    connect(ui->userSkyQuality, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &SubExposureCalculatorPOC::handleUserAdjustment);
    connect(ui->noiseTolerance, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &SubExposureCalculatorPOC::handleUserAdjustment);

    connect(ui->filterBandwidth, QOverload<int>::of(&QSpinBox::valueChanged), this, &SubExposureCalculatorPOC::handleUserAdjustment);

    connect(ui->isoDiscreteSelector, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &SubExposureCalculatorPOC::handleUserAdjustment);


// Hide the gain selector frames (until a camera is selected)

    ui->gainSelectionFixedFrame->setEnabled(false); ui->gainSelectionFixedFrame->setVisible(false);
    ui->gainSelectionNormalFrame->setEnabled(false); ui->gainSelectionNormalFrame->setVisible(false);
    ui->gainSelectionISODiscreteFrame->setEnabled(false);ui->gainSelectionISODiscreteFrame->setVisible(false);
}

SubExposureCalculatorPOC::~SubExposureCalculatorPOC()
{
    delete ui;
}

int SubExposureCalculatorPOC::getGainSelection(OptimalExposure::GainSelectionType aGainSelectionType){
    int aSelectedGain = 0;
    switch(aGainSelectionType) {

    case OptimalExposure::GAIN_SELECTION_TYPE_NORMAL:
        aSelectedGain = ui->gainSlider->value();
        break;


    case OptimalExposure::GAIN_SELECTION_TYPE_ISO_DISCRETE:
        qDebug() << " iso selector text: " << ui->isoDiscreteSelector->currentText();
        aSelectedGain = ui->isoDiscreteSelector->currentText().toInt();
        break;

    case OptimalExposure::GAIN_SELECTION_TYPE_FIXED:
//            qDebug() << "Fixed read-noise cameras still under development";
        aSelectedGain = 9;
        break;


    }

    return(aSelectedGain);
}

void SubExposureCalculatorPOC::realignGainSlider(){
    /*
        Needs a better design... trying to keep the slider aligned with the graph,
        but the right margin must be shifted and width must be extended
        because the plot can shift on x-axis when y-axis info is updated.
    */


//        qDebug() << "plot x " << ui->qCustomPlotSubExposure->x();

//        qDebug() << "xAxis bottomLeft x" << ui->qCustomPlotSubExposure->xAxis->axisRect()->bottomLeft().x();
//        qDebug() << "xAxis bottomLeft y" << ui->qCustomPlotSubExposure->xAxis->axisRect()->bottomLeft().y();

//        qDebug() << "xAxis bottomRight x " << ui->qCustomPlotSubExposure->xAxis->axisRect()->bottomRight().x();
//        qDebug() << "xAxis bottomRight y " << ui->qCustomPlotSubExposure->xAxis->axisRect()->bottomRight().y();
//        // qDebug() << "gain slider x " << ui->gainSlider->x();
//        qDebug() << "gain slider x,y " << ui->gainSlider->x() << "," << ui->gainSlider->y();


        int sliderHandleMargin = 8;
        ui->gainSlider->setGeometry(
            ui->qCustomPlotSubExposure->xAxis->axisRect()->bottomLeft().x() + (-sliderHandleMargin/2),
            6,
            ui->qCustomPlotSubExposure->xAxis->axisRect()->bottomRight().x() - ui->qCustomPlotSubExposure->xAxis->axisRect()->bottomLeft().x() + sliderHandleMargin,
            20);

}


void SubExposureCalculatorPOC::handleUserAdjustment(){

    // This test for enabled was needed because dynamic changes to a
    // combo box during initialization of the calculator were firing
    // this method and prematurely triggering a calculation which was
    // crashing because the initialization was incomplete.

    if(ui->exposureCalculatorMockUp->isEnabled()){


        // Recalculate and refresh the graph, with changed inputs from the ui
        QString aSelectedImagingSensor = ui->imagingSensorSelector->itemText(ui->imagingSensorSelector->currentIndex());
        double aFocalRatioValue = ui->indiFocalRatio->value();

        double aSkyQualityValue = ui->userSkyQuality->value();
        qDebug() << "\ta selected Sky Quality: " << aSkyQualityValue;

        double aNoiseTolerance = ui->noiseTolerance->value();

        // double aFilterCompensationValue = 1.0;
        double aFilterCompensationValue = ((double)ui->filterBandwidth->value() / (double)300);

        int aSelectedGainValue = getGainSelection(anOptimalSubExposureCalculator->getImagingSensorData().getGainSelectionType());


        // double aSelectedGainValue = ui->gainSlider->value();
        qDebug() << "\ta selected gain: " << aSelectedGainValue;

        calculateSubExposure(aNoiseTolerance, aSkyQualityValue, aFocalRatioValue, aFilterCompensationValue, aSelectedGainValue);
    }
}



void SubExposureCalculatorPOC::applyInitialInputs()
{

    ui->exposureCalculatorMockUp->setEnabled(false);

    QString aSelectedImagingSensorName = ui->imagingSensorSelector->itemText(ui->imagingSensorSelector->currentIndex());
    double aFocalRatioValue = ui->indiFocalRatio->value();
    double aSkyQualityValue = ui->indiSkyQuality->value();
    // sync-up the user SQM slider.
    ui->userSkyQuality-> setValue(aSkyQualityValue);

    double aNoiseTolerance = ui->noiseTolerance->value();

    // double aFilterCompensationValue = 1.0;
    // double aFilterCompensationValue = ui->filterSelection->itemData(ui->filterSelection->currentIndex()).toDouble();
    double aFilterCompensationValue = ((double)ui->filterBandwidth->value() / (double)300);

    initializeSubExposureCalculator(aNoiseTolerance, aSkyQualityValue, aFocalRatioValue, aFilterCompensationValue, aSelectedImagingSensorName);

    int aSelectedGainValue = ui->gainSlider->value();

    calculateSubExposure(aNoiseTolerance, aSkyQualityValue, aFocalRatioValue, aFilterCompensationValue, aSelectedGainValue);

    ui->exposureCalculatorMockUp->setEnabled(true);

}

void replotSubExposureEnvelope(Ui::SubExposureCalculatorPOC *ui, OptimalExposure::OptimalSubExposureCalculator *anOptimalSubExposureCalculator, OptimalExposure::OptimalExposureDetail *subExposureDetail){

    OptimalExposure::CameraExposureEnvelope aCameraExposureEnvelope = anOptimalSubExposureCalculator->calculateCameraExposureEnvelope();
    qDebug() << "Exposure Envelope has a set of: " << aCameraExposureEnvelope.getASubExposureVector().size();
    qDebug() << "Exposure Envelope has a minimum Exposure Time of " << aCameraExposureEnvelope.getExposureTimeMin();
    qDebug() << "Exposure Envelope has a maximum Exposure Time of " << aCameraExposureEnvelope.getExposureTimeMax();

    // anOptimalSubExposureCalculator->getImagingSensorData()

    // Reset the graph axis (But maybe this was not necessary,
    ui->qCustomPlotSubExposure->xAxis->setRange(anOptimalSubExposureCalculator->getImagingSensorData().getGainMin(),anOptimalSubExposureCalculator->getImagingSensorData().getGainMax());
    // But for the exposure yAxis include a bit of a margin so that data is not encoaching on the axis.
    ui->qCustomPlotSubExposure->yAxis->setRange(aCameraExposureEnvelope.getExposureTimeMin()-10,aCameraExposureEnvelope.getExposureTimeMax()+10);
    ui->qCustomPlotSubExposure->replot();

    // Prepare for the exposure line plot, move the data to parallel arrays for the custom plotter
    QVector<double> gain(aCameraExposureEnvelope.getASubExposureVector().size()), exposuretime(aCameraExposureEnvelope.getASubExposureVector().size());
    for(int index=0; index < aCameraExposureEnvelope.getASubExposureVector().size(); index++){
        OptimalExposure::CalculatedGainSubExposureTime aGainExposureTime = aCameraExposureEnvelope.getASubExposureVector()[index];
        gain[index] = (double)aGainExposureTime.getSubExposureGain();
        exposuretime[index] = aGainExposureTime.getSubExposureTime();
    }
    ui->qCustomPlotSubExposure->graph()->data()->clear();

    ui->qCustomPlotSubExposure->graph(0)->setData(gain,exposuretime);


    // Also add a graph with a vertical line to show the selected gain...
    ui->qCustomPlotSubExposure->addGraph();


    QVector<double> selectedExposureX(2), selectedExposureY(2);
    selectedExposureX[0] = subExposureDetail->getSelectedGain();
    selectedExposureY[0] = 0;
    selectedExposureX[1] = subExposureDetail->getSelectedGain();
    selectedExposureY[1] = subExposureDetail->getSubExposureTime();
    ui->qCustomPlotSubExposure->graph(1)->setData(selectedExposureX,selectedExposureY);

    QPen penExposureEnvelope;
    penExposureEnvelope.setWidth(1);
    penExposureEnvelope.setColor(QColor(0,180,180));
    ui->qCustomPlotSubExposure->graph(0)->setPen(penExposureEnvelope);

    QPen penSelectedExposure;
    penSelectedExposure.setWidth(1);
    penSelectedExposure.setColor(QColor(180,0,0));
    ui->qCustomPlotSubExposure->graph(1)->setPen(penSelectedExposure);

    ui->qCustomPlotSubExposure->graph(1)->setScatterStyle(QCPScatterStyle::ssCircle);

    // extend the x-axis slightly so that the markers aren't hidden at the extreme edges
    ui->qCustomPlotSubExposure->xAxis->setRange(anOptimalSubExposureCalculator->getImagingSensorData().getGainMin()-5, anOptimalSubExposureCalculator->getImagingSensorData().getGainMax()+5);
    // force the y-axis to start at 0, (sometimes the auto rescale was making the y-axis range start a negative value
    ui->qCustomPlotSubExposure->yAxis->setRange(0, aCameraExposureEnvelope.getExposureTimeMax());

    ui->qCustomPlotSubExposure->graph()->rescaleAxes(true);
    ui->qCustomPlotSubExposure->replot();

}

void SubExposureCalculatorPOC::initializeSubExposureCalculator(double aNoiseTolerance, double aSkyQualityValue, double aFocalRatioValue, double aFilterCompensationValue, QString aSelectedImagingSensorName)
{
    qDebug() << "initializeSubExposureComputer";
    qDebug() << "\taNoiseTolerance: " << aNoiseTolerance;
    qDebug() << "\taSkyQualityValue: " << aSkyQualityValue;
    qDebug() << "\taFocalRatioValue: " << aFocalRatioValue;
    qDebug() << "\taFilterCompensation: " << aFilterCompensationValue;
    qDebug() << "\taSelectedImagingSensor: " << aSelectedImagingSensorName;

    QVector<int> *aGainSelectionRange = new QVector<int>();
    QVector<OptimalExposure::SensorGainReadNoise> *aSensorGainReadNoiseVector = new QVector<OptimalExposure::SensorGainReadNoise>();
    // Iniitalize with some default values before attempting to load from file
    anImagingSensorData = new OptimalExposure::ImagingSensorData(aSelectedImagingSensorName, OptimalExposure::SENSORTYPE_COLOR, OptimalExposure::GAIN_SELECTION_TYPE_NORMAL, *aGainSelectionRange, *aSensorGainReadNoiseVector);

    // Load sensor data from file
    OptimalExposure::FileUtilitySensorData::readSensorDataFile(aSelectedImagingSensorName, anImagingSensorData);



    qDebug() << "Loaded Imaging Sensor Data for " + anImagingSensorData->getSensorId();
    qDebug() << "Sensor Gain Selection Type " +  QString::number(anImagingSensorData->getGainSelectionType());

    qDebug() << "Sensor Gain Read-Noise Vector Size " + QString::number(anImagingSensorData->getSensorGainReadNoiseVector().size());

    switch( anImagingSensorData->getGainSelectionType() ) {
    case OptimalExposure::GAIN_SELECTION_TYPE_FIXED:
        qDebug() << "Gain Selection Type: GAIN_SELECTION_TYPE_FIXED";

        ui->gainSelectionISODiscreteFrame->setEnabled(false);ui->gainSelectionISODiscreteFrame->setVisible(false);
        ui->gainSelectionNormalFrame->setEnabled(false); ui->gainSelectionNormalFrame->setVisible(false);
        ui->gainSelectionFixedFrame->setEnabled(true); ui->gainSelectionFixedFrame->setVisible(true);

        break;

    case OptimalExposure::GAIN_SELECTION_TYPE_ISO_DISCRETE:
        qDebug() << "Gain Selection Type: GAIN_SELECTION_TYPE_ISO_DISCRETE";
        ui->gainSelectionFixedFrame->setEnabled(false); ui->gainSelectionFixedFrame->setVisible(false);
        ui->gainSelectionNormalFrame->setEnabled(false); ui->gainSelectionNormalFrame->setVisible(false);
        ui->gainSelectionISODiscreteFrame->setEnabled(false);ui->gainSelectionISODiscreteFrame->setVisible(false);

        qDebug() << "ui->isoDiscreteSelector->isEnabled(): " << ui->isoDiscreteSelector->isEnabled();
        qDebug() << "ui->noiseTolerance->isEnabled(): " << ui->noiseTolerance->isEnabled();

        ui->isoDiscreteSelector->clear();
        // Load the ISO Combo from the camera data
        foreach(int isoSetting, anImagingSensorData->getGainSelectionRange()) {
            ui->isoDiscreteSelector->addItem(QString::number(isoSetting));
        }
        ui->isoDiscreteSelector->setCurrentIndex(0);

        qDebug() << "Sensor Data Gain min " +  QString::number(anImagingSensorData->getGainMin());
        qDebug() << "Sensor Data Gain max " +  QString::number(anImagingSensorData->getGainMax());

        ui->gainSelectionISODiscreteFrame->setEnabled(true);ui->gainSelectionISODiscreteFrame->setVisible(true);

        break;

    case OptimalExposure::GAIN_SELECTION_TYPE_NORMAL:
        qDebug() << "Gain Selection Type: GAIN_SELECTION_TYPE_NORMAL";
        ui->gainSelectionFixedFrame->setEnabled(false); ui->gainSelectionFixedFrame->setVisible(false);
        ui->gainSelectionISODiscreteFrame->setEnabled(false);ui->gainSelectionISODiscreteFrame->setVisible(false);
        ui->gainSelectionNormalFrame->setEnabled(true); ui->gainSelectionNormalFrame->setVisible(true);
        qDebug() << "Sensor Data Gain min " +  QString::number(anImagingSensorData->getGainMin());
        qDebug() << "Sensor Data Gain max " +  QString::number(anImagingSensorData->getGainMax());
        break;

    }

    anOptimalSubExposureCalculator = new OptimalExposure::OptimalSubExposureCalculator(aNoiseTolerance, aSkyQualityValue, aFocalRatioValue, aFilterCompensationValue, *anImagingSensorData);

    qDebug() << "Calculating... ";
    qDebug() << "A Noise Tolerance " << anOptimalSubExposureCalculator->getANoiseTolerance();
    qDebug() << "A Sky Quality " << anOptimalSubExposureCalculator->getASkyQuality();

    qDebug() << "A Focal Ratio " << anOptimalSubExposureCalculator->getAFocalRatio();
    qDebug() << "A Filter Compensation Value (ignored): " << anOptimalSubExposureCalculator->getAFilterCompensation();

    qDebug() << "A Sensor Gain Min " << anOptimalSubExposureCalculator->getImagingSensorData().getGainMin();
    qDebug() << "A Sensor Gain Max " << anOptimalSubExposureCalculator->getImagingSensorData().getGainMax();

}



void SubExposureCalculatorPOC::calculateSubExposure(double aNoiseTolerance, double aSkyQualityValue, double aFocalRatioValue, double aFilterCompensationValue, int aSelectedGainValue)
{

    anOptimalSubExposureCalculator->setANoiseTolerance(aNoiseTolerance);
    anOptimalSubExposureCalculator->setASkyQuality(aSkyQualityValue);
    anOptimalSubExposureCalculator->setAFocalRatio(aFocalRatioValue);  // this should notbe necessary since the UI panel has not widget for it.
    anOptimalSubExposureCalculator->setAFilterCompensation(aFilterCompensationValue);
    anOptimalSubExposureCalculator->setASelectedGain(aSelectedGainValue);


    qDebug() << "initializeSubExposureComputer";
    qDebug() << "\taNoiseTolerance: " << aNoiseTolerance;
    qDebug() << "\taSkyQualityValue: " << aSkyQualityValue;
    qDebug() << "\taFocalRatioValue: " << aFocalRatioValue;
    qDebug() << "\taFilterCompensation: (ignored) " << aFilterCompensationValue;
    qDebug() << "\taSelectedGainValue: " << aSelectedGainValue;

    anOptimalSubExposureCalculator->setAFilterCompensation(aFilterCompensationValue);

    qDebug() << "Calculating... ";
    qDebug() << "A Noise Tolerance " << anOptimalSubExposureCalculator->getANoiseTolerance();
    qDebug() << "A Sky Quality " << anOptimalSubExposureCalculator->getASkyQuality();

    qDebug() << "A Focal Ratio " << anOptimalSubExposureCalculator->getAFocalRatio();
    qDebug() << "A Filter Compensation Value (ignored): " << anOptimalSubExposureCalculator->getAFilterCompensation();

    qDebug() << "A Sensor Gain Min " << anOptimalSubExposureCalculator->getImagingSensorData().getGainMin();
    qDebug() << "A Sensor Gain Max " << anOptimalSubExposureCalculator->getImagingSensorData().getGainMax();


    OptimalExposure::CameraExposureEnvelope aCameraExposureEnvelope = anOptimalSubExposureCalculator->calculateCameraExposureEnvelope();
    qDebug() << "Exposure Envelope has a set of: " << aCameraExposureEnvelope.getASubExposureVector().size();
    qDebug() << "Exposure Envelope has a minimum Exposure Time of " << aCameraExposureEnvelope.getExposureTimeMin();
    qDebug() << "Exposure Envelope has a maximum Exposure Time of " << aCameraExposureEnvelope.getExposureTimeMax();



    OptimalExposure::OptimalExposureDetail subExposureDetail = anOptimalSubExposureCalculator->calculateSubExposureDetail(aSelectedGainValue);
    // Get the exposure details into the ui
    //ui->exposureCalculatonResult.

    replotSubExposureEnvelope(ui, anOptimalSubExposureCalculator, &subExposureDetail);
    if(ui->gainSlider->isEnabled()){
        realignGainSlider();
        ui->gainSlider->setMaximum(anOptimalSubExposureCalculator->getImagingSensorData().getGainMax());
        ui->gainSlider->setMinimum(anOptimalSubExposureCalculator->getImagingSensorData().getGainMin());
    }

    QTableWidget *exposureShotResult = ui->exposureShotResult;
    exposureShotResult->verticalHeader()->setVisible(false);
    exposureShotResult->horizontalHeader()->setVisible(false);
    exposureShotResult->setColumnCount(2);
    exposureShotResult->setRowCount(5);

    exposureShotResult->setItem(0,0, new QTableWidgetItem("Gain"));
    exposureShotResult->setItem(0,1, new QTableWidgetItem(QString::number(aSelectedGainValue)));
    exposureShotResult->setRowHeight(0,22);

    exposureShotResult->setItem(1,0, new QTableWidgetItem("Exposure Time (seconds)"));
    exposureShotResult->setItem(1,1, new QTableWidgetItem(QString::number(subExposureDetail.getSubExposureTime(), 'f', 2)));
    exposureShotResult->setRowHeight(1,22);

    exposureShotResult->setItem(2,0, new QTableWidgetItem("Exposure Pollution Electrons"));
    exposureShotResult->setItem(2,1, new QTableWidgetItem(QString::number(subExposureDetail.getExposurePollutionElectrons(), 'f', 2)));
    exposureShotResult->setRowHeight(2,22);

    exposureShotResult->setItem(3,0, new QTableWidgetItem("Exposure Shot Noise"));
    exposureShotResult->setItem(3,1, new QTableWidgetItem(QString::number(subExposureDetail.getExposureShotNoise(), 'f', 2)));
    exposureShotResult->setRowHeight(3,22);

    exposureShotResult->setItem(4,0, new QTableWidgetItem("Exposure Total Noise"));
    exposureShotResult->setItem(4,1, new QTableWidgetItem(QString::number(subExposureDetail.getExposureTotalNoise(), 'f', 2)));
    exposureShotResult->setRowHeight(4,22);

    exposureShotResult->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    qDebug() << "Exposure Pollution Electrons: " << subExposureDetail.getExposurePollutionElectrons();
    qDebug() << "Exposure Shot Noise: " << subExposureDetail.getExposureShotNoise();
    qDebug() << "Exposure Total Noise: " << subExposureDetail.getExposureTotalNoise();


    QTableWidget *resultStackTable = ui->exposureStackResult;
    resultStackTable->setColumnCount(5);
    resultStackTable->verticalHeader()->setVisible(false);

    QStringList stackDetailHeaders;
        stackDetailHeaders << "Planned Hours" << "Exposure Count" << "Stack Time" << "Stack Noise" << "Ratio";
        resultStackTable->setHorizontalHeaderLabels(stackDetailHeaders);
        resultStackTable->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter | (Qt::Alignment)Qt::TextWordWrap);
        resultStackTable->horizontalHeader()->setFixedHeight(32);

    int stackSummarySize =  subExposureDetail.getStackSummary().size();
    resultStackTable->setRowCount(stackSummarySize);

    for(int stackSummaryIndex = 0; stackSummaryIndex < stackSummarySize; stackSummaryIndex++){
        OptimalExposure::OptimalExposureStack anOptimalExposureStack = subExposureDetail.getStackSummary()[stackSummaryIndex];

        resultStackTable->setItem(stackSummaryIndex,0, new QTableWidgetItem(QString::number(anOptimalExposureStack.getPlannedTime())));
        resultStackTable->item(stackSummaryIndex,0)->setTextAlignment(Qt::AlignCenter);

        resultStackTable->setItem(stackSummaryIndex,1, new QTableWidgetItem(QString::number(anOptimalExposureStack.getExposureCount())));
        resultStackTable->item(stackSummaryIndex,1)->setTextAlignment(Qt::AlignRight);

        resultStackTable->setItem(stackSummaryIndex,2, new QTableWidgetItem(QString::number(anOptimalExposureStack.getStackTime())));
        resultStackTable->item(stackSummaryIndex,2)->setTextAlignment(Qt::AlignRight);

        resultStackTable->setItem(stackSummaryIndex,3, new QTableWidgetItem(QString::number(anOptimalExposureStack.getStackTotalNoise(), 'f', 2)));
        resultStackTable->item(stackSummaryIndex,3)->setTextAlignment(Qt::AlignRight);

        double ratio = anOptimalExposureStack.getStackTime() / anOptimalExposureStack.getStackTotalNoise();
        resultStackTable->setItem(stackSummaryIndex,4, new QTableWidgetItem(QString::number(ratio, 'f', 3)));
        resultStackTable->item(stackSummaryIndex,4)->setTextAlignment(Qt::AlignRight);

        resultStackTable->setRowHeight(stackSummaryIndex, 22);

        qDebug() << "Stack info: Hours: " << anOptimalExposureStack.getPlannedTime()
                 << " Exposure Count: " << anOptimalExposureStack.getExposureCount()
                 << " Stack Time: " << anOptimalExposureStack.getStackTime()
                 << " Stack Total Noise: " << anOptimalExposureStack.getStackTotalNoise();
    }


    resultStackTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

}


