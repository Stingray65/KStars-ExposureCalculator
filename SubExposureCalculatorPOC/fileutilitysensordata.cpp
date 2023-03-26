#include <QDebug>
#include <QFile>
#include <QIODevice>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include "fileutilitysensordata.h"
#include "imagingsensordata.h"
#include "sensorgainreadnoise.h"


QString ImageSensorDataPath = "../ImageSensorData/";

int OptimalExposure::FileUtilitySensorData::readSensorDataFile(QString sensorId, OptimalExposure::ImagingSensorData *anImagingSensorData){

    QVector<OptimalExposure::SensorGainReadNoise> *aSensorGainReadNoiseVector = new QVector<OptimalExposure::SensorGainReadNoise>();

    QString aSensorDataFile = ImageSensorDataPath + sensorId;  // this should be a filename with a lowercase extension ".xml".
    qDebug() << "Opening... " + aSensorDataFile;

    QFile file(aSensorDataFile);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug() << "Cannot open file for reading " << file.errorString();
        exit(0);
    }

    qDebug() << "Reading... " + aSensorDataFile;
    QXmlStreamReader xmlReader(&file);

    if (xmlReader.readNextStartElement()) {
        if (xmlReader.name() == "ACamera"){
            qDebug() << "Found aCamera Element...";

            while(xmlReader.readNextStartElement()){
                if (xmlReader.name() == "SensorId"){
                    qDebug() << "Found aSensorId Element...";
                    QString aSensorIdString = xmlReader.readElementText();
                    qDebug() << "Read element text..." + aSensorIdString;
                    anImagingSensorData->setSensorId(aSensorIdString);
                }

                if (xmlReader.name() == "SensorType"){
                    qDebug() << "Found SensorType Element...";
                    QString aSensorType = xmlReader.readElementText();
                    qDebug() << "Read element text..." + aSensorType;
                    if(aSensorType == "COLOR") anImagingSensorData->setSensorType(OptimalExposure::SENSORTYPE_COLOR);
                    if(aSensorType == "MONOCHROME") anImagingSensorData->setSensorType(OptimalExposure::SENSORTYPE_MONOCHROME);
                }

                if (xmlReader.name() == "GainSelectionType"){
                    qDebug() << "Found GainSelectionType Element...";
                    QString aGainSelectionType = xmlReader.readElementText();
                    qDebug() << "Read element text..." + aGainSelectionType;
                    if(aGainSelectionType == "NORMAL") anImagingSensorData->setGainSelectionType(OptimalExposure::GAIN_SELECTION_TYPE_NORMAL);
                    if(aGainSelectionType == "ISO_DISCRETE") anImagingSensorData->setGainSelectionType(OptimalExposure::GAIN_SELECTION_TYPE_ISO_DISCRETE);
                    if(aGainSelectionType == "FIXED") anImagingSensorData->setGainSelectionType(OptimalExposure::GAIN_SELECTION_TYPE_FIXED);
                }


                // GainSelection collection  // For GAIN_SELECTION_TYPE_NORMAL the collection represents min and max, for GAIN_SELECTION_TYPE_ISO_DISCRETE, the collection is the discrete list of of gain/iso values.
                // Note that the data available for GainReadNoiseValue may not match a camera discrete gain/iso values, so this collection resolves that.

                if (xmlReader.name() == "SensorGainSelections"){
                    QVector<int> *aGainSelectionVector = new QVector<int>();

                    int aGainSelection = 0;
                    while(xmlReader.readNext() && !xmlReader.isEndElement() && !xmlReader.isEndDocument()){
                         qDebug() << "xmlRead.name() = " <<  xmlReader.name();
                         if (xmlReader.name() == "GainSelection"){
                             qDebug() << "Found GainSelection Element...";
                             QString aGainSelectionString = xmlReader.readElementText();
                             aGainSelection = aGainSelectionString.toInt();
                             qDebug() << "a Found GainSelection text: " << aGainSelection;

                             qDebug() << "Adding GainSelection to Vector " << QString::number(aGainSelection);

                             aGainSelectionVector->push_back(aGainSelection);
                         }
                    }
                    anImagingSensorData->setGainSelectionRange(*aGainSelectionVector);
                }

                // SensorGainReadNoise collection
                if (xmlReader.name() == "SensorGainReadNoise"){
                    qDebug() << "Found SensorGainReadNoise Element...";
                    // QString aSensorGainReadNoise = xmlReader.readElementText();
                    // qDebug() << "Read element ..." + aSensorGainReadNoise;

                    // Iterate for Gain Read-Noise data
                    int aGain = 0;
                    double aReadNoise = 0.0;
                    while(xmlReader.readNext() && !xmlReader.isEndDocument()){ //while(xmlReader.readNextStartElement()){
                        qDebug() << xmlReader.name();
                        if (xmlReader.name() == "GainReadNoiseValue"){
                            qDebug() << "Found GainReadNoiseValue Element...";
                        }

                        if (xmlReader.name() == "Gain"){
                            qDebug() << "Found Gain Element...";
                            QString aGainString = xmlReader.readElementText();
                            aGain = aGainString.toInt();
                            qDebug() << "a Found Gain text: " << aGain;
                        }

                        if (xmlReader.name() == "ReadNoise"){
                            qDebug() << "Found ReadNoise Element...";
                            QString aReadNoiseString = xmlReader.readElementText();
                            aReadNoise = aReadNoiseString.toDouble();
                            qDebug() << "a Found ReadNoise text: " << aReadNoise;

                            // Add this to a vector
                            qDebug() << "Adding Gain Read-Noise to Vector " << QString::number(aGain) << " " << QString::number(aReadNoise) ;
                            aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(aGain, aReadNoise)));

                        }
                    }
                    anImagingSensorData->setSensorGainReadNoiseVector(*aSensorGainReadNoiseVector);
                }
            }
            qDebug() << "Read xml data for " + anImagingSensorData->getSensorId();

        }else{
            qDebug() << "Read Failed";
            xmlReader.raiseError(QObject::tr("Incorrect file"));
        }
    }else{
         qDebug() << "Read Iniial Element Failed,";
    }


    file.close();
    return 0;
}

int OptimalExposure::FileUtilitySensorData::writeSensorDataFile(OptimalExposure::ImagingSensorData *anImagingSensorData){


    QString aSensorDataFile = ImageSensorDataPath + anImagingSensorData->getSensorId();

    if(!aSensorDataFile.endsWith(".xml")){
        aSensorDataFile+=".xml";
    }

    QFile file(aSensorDataFile);
    if(!file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text)){
        qDebug() << "Cannot open file for writing " << file.errorString();
        exit(0);
    }

    QXmlStreamWriter xmlWriter(&file);
    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument();

    xmlWriter.writeStartElement("ACamera");

    xmlWriter.writeTextElement("SensorId", anImagingSensorData->getSensorId() );

    // xmlWriter.writeTextElement("SensorType", QString("MONOCHROME"));

    switch(anImagingSensorData->getSensorType()){
    case OptimalExposure::SENSORTYPE_MONOCHROME:
        xmlWriter.writeTextElement("SensorType", QString("MONOCHROME"));
        break;
    case OptimalExposure::SENSORTYPE_COLOR:
        xmlWriter.writeTextElement("SensorType", QString("COLOR"));
        break;
    }

    switch(anImagingSensorData->getGainSelectionType()){
    case OptimalExposure::GAIN_SELECTION_TYPE_NORMAL:
        xmlWriter.writeTextElement("GainSelectionType", QString("NORMAL"));
        break;
    case OptimalExposure::GAIN_SELECTION_TYPE_ISO_DISCRETE:
        xmlWriter.writeTextElement("GainSelectionType", QString("ISO_DISCRETE"));
        break;
    case OptimalExposure::GAIN_SELECTION_TYPE_FIXED:
        xmlWriter.writeTextElement("GainSelectionType", QString("FIXED"));
        break;
    }

    xmlWriter.writeStartElement("SensorGainSelections");
    QVector<int> aGainSelectionRange = anImagingSensorData->getGainSelectionRange();
    for(int gs=0; gs<aGainSelectionRange.count(); gs++){
        // xmlWriter.writeStartElement("GainSelection");
        xmlWriter.writeTextElement("GainSelection", QString::number(anImagingSensorData->getGainSelectionRange()[gs]));
        // xmlWriter.writeEndElement();
    }
    xmlWriter.writeEndElement();

    // Iterate through values of anImagingSensorData->sensorGainReadNoiseVector
    xmlWriter.writeStartElement("SensorGainReadNoise");
    QVector<OptimalExposure::SensorGainReadNoise> aSensorGainReadNoiseVector = anImagingSensorData->getSensorGainReadNoiseVector();

    for(QVector<OptimalExposure::SensorGainReadNoise>::iterator rn = aSensorGainReadNoiseVector.begin(); rn != aSensorGainReadNoiseVector.end(); ++rn){
        xmlWriter.writeStartElement("GainReadNoiseValue");
        xmlWriter.writeTextElement("Gain", QString::number(rn->getGain()));
        xmlWriter.writeTextElement("ReadNoise", QString::number(rn->getReadNoise()));
        xmlWriter.writeEndElement();
    }
    xmlWriter.writeEndElement();

    xmlWriter.writeEndElement();
    xmlWriter.writeEndDocument();
    file.close();

    return 0;

}

void OptimalExposure::FileUtilitySensorData::buildSensorDataFile()
{

    // Make each camera sensor read noise data file

    // re-using these for each camera instance
    QString cameraName;
    QVector<OptimalExposure::SensorGainReadNoise> *aSensorGainReadNoiseVector;
    QVector<int> *gainSelection;
    OptimalExposure::ImagingSensorData *anImagingSensorData;

/*
    cameraName = "ZWOASI-6200MM";
    aSensorGainReadNoiseVector = new QVector<OptimalExposure::SensorGainReadNoise>();

    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(0, 3.6)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(50, 3.5)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(99, 3.38)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(100, 1.55)));  // pronounced step
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(150, 1.5)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(200, 1.45)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(250, 1.42)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(300, 1.40)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(350, 1.41)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(400, 1.32)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(450, 1.39)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(470, 1.25)));

    gainSelection = new QVector<int>( { 0, 470 } );

    qDebug() << "aSensorGainReadNoiseVector size: " << aSensorGainReadNoiseVector->size();
    anImagingSensorData = new OptimalExposure::ImagingSensorData(cameraName, OptimalExposure::SENSORTYPE_MONOCHROME, OptimalExposure::GAIN_SELECTION_TYPE_NORMAL, *gainSelection, *aSensorGainReadNoiseVector);
    OptimalExposure::FileUtilitySensorData::writeSensorDataFile(anImagingSensorData);


    cameraName = "ZWOASI-071MCPro";
    aSensorGainReadNoiseVector = new QVector<OptimalExposure::SensorGainReadNoise>();
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(0, 3.28)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(50, 2.78)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(100, 2.58)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(150, 2.39)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(200, 2.29)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(250, 2.27)));

    gainSelection = new QVector<int>( { 0, 250 } );
    anImagingSensorData = new OptimalExposure::ImagingSensorData(cameraName, OptimalExposure::SENSORTYPE_COLOR, OptimalExposure::GAIN_SELECTION_TYPE_NORMAL, *gainSelection, *aSensorGainReadNoiseVector);
    OptimalExposure::FileUtilitySensorData::writeSensorDataFile(anImagingSensorData);

    cameraName = "ZWOASI-1600MM";
    aSensorGainReadNoiseVector = new QVector<OptimalExposure::SensorGainReadNoise>();
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(0, 3.62)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(50, 2.5)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(100, 1.85)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(150, 1.72)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(200, 1.40)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(250, 1.32)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(300, 1.30)));

    gainSelection = new QVector<int>( { 0, 300 } );
    anImagingSensorData = new OptimalExposure::ImagingSensorData(cameraName, OptimalExposure::SENSORTYPE_MONOCHROME, OptimalExposure::GAIN_SELECTION_TYPE_NORMAL, *gainSelection, *aSensorGainReadNoiseVector);
    OptimalExposure::FileUtilitySensorData::writeSensorDataFile(anImagingSensorData);

    cameraName = "ZWOASI-224MC";
    aSensorGainReadNoiseVector = new QVector<OptimalExposure::SensorGainReadNoise>();
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(0, 3.09)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(20, 2.71)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(59, 2.26)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(60, 1.6)));    // pronounced step
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(70, 1.54)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(100, 1.34)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(150, 1.14)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(250, 0.93)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(300, 0.86)));

    gainSelection = new QVector<int>( { 0, 300 } );
    anImagingSensorData = new OptimalExposure::ImagingSensorData(cameraName, OptimalExposure::SENSORTYPE_COLOR, OptimalExposure::GAIN_SELECTION_TYPE_NORMAL, *gainSelection, *aSensorGainReadNoiseVector);
    OptimalExposure::FileUtilitySensorData::writeSensorDataFile(anImagingSensorData);

    cameraName = "ZWOASI-120MC";
    aSensorGainReadNoiseVector = new QVector<OptimalExposure::SensorGainReadNoise>();
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(0, 6.35)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(10, 6.41)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(15, 6.5)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(16, 4.63))); // pronounced step
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(20, 4.67))); // a rise after the step down
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(31, 4.7)));  // another step
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(32, 3.87)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(47, 3.95))); // another rise after the step down
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(48, 3.7)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(63, 3.68))); // another step
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(64, 3.54)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(75, 3.66))); // another rise after a step down
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(87, 3.62)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(100, 3.62)));

    gainSelection = new QVector<int>( { 0, 100 } );
    anImagingSensorData = new OptimalExposure::ImagingSensorData(cameraName, OptimalExposure::SENSORTYPE_COLOR, OptimalExposure::GAIN_SELECTION_TYPE_NORMAL, *gainSelection, *aSensorGainReadNoiseVector);
    OptimalExposure::FileUtilitySensorData::writeSensorDataFile(anImagingSensorData);

    cameraName = "ZWOASI-178MC";
    aSensorGainReadNoiseVector = new QVector<OptimalExposure::SensorGainReadNoise>();
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(0, 2.23)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(50, 1.92)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(100, 1.74)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(150, 1.58)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(200, 1.45)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(250, 1.39)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(300, 1.38)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(400, 1.35)));

    gainSelection = new QVector<int>( { 0, 400 } );
    anImagingSensorData = new OptimalExposure::ImagingSensorData(cameraName, OptimalExposure::SENSORTYPE_COLOR, OptimalExposure::GAIN_SELECTION_TYPE_NORMAL, *gainSelection, *aSensorGainReadNoiseVector);
    OptimalExposure::FileUtilitySensorData::writeSensorDataFile(anImagingSensorData);


    cameraName = "QHY533M";
    aSensorGainReadNoiseVector = new QVector<OptimalExposure::SensorGainReadNoise>();
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(0,3.34)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(5,3.3)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(10,3.32)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(15,3.4)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(20,3.3)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(25,3.22)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(30,3.19)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(35,3.09)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(40,3.18)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(45,3.12)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(50,3.05)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(55,2.93)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(60,1.66))); // pronounced step
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(65,1.6)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(70,1.56)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(75,1.54)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(80,1.51)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(85,1.45)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(90,1.4)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(95,1.34)));

    gainSelection = new QVector<int>( { 0, 95 } );
    anImagingSensorData = new OptimalExposure::ImagingSensorData(cameraName, OptimalExposure::SENSORTYPE_MONOCHROME, OptimalExposure::GAIN_SELECTION_TYPE_NORMAL, *gainSelection, *aSensorGainReadNoiseVector);
    OptimalExposure::FileUtilitySensorData::writeSensorDataFile(anImagingSensorData);

    cameraName = "ZWOASI-183MCPro";  // The same read-noise is used in both Mono and Color
    aSensorGainReadNoiseVector = new QVector<OptimalExposure::SensorGainReadNoise>();
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(0,3.00)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(50,2.6)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(100,2.22)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(150,2.02)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(200,1.84)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(250,1.75)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(270,1.68)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(300,1.58)));

    gainSelection = new QVector<int>( { 0, 300 } );
    anImagingSensorData = new OptimalExposure::ImagingSensorData(cameraName, OptimalExposure::SENSORTYPE_COLOR, OptimalExposure::GAIN_SELECTION_TYPE_NORMAL, *gainSelection, *aSensorGainReadNoiseVector);
    OptimalExposure::FileUtilitySensorData::writeSensorDataFile(anImagingSensorData);

    cameraName = "Moravian-C3";
    aSensorGainReadNoiseVector = new QVector<OptimalExposure::SensorGainReadNoise>();
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(0, 3.51)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(2749, 3.15)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(2750, 1.46)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(4030, 1.39)));
    gainSelection = new QVector<int>( { 0, 4030 } );
    anImagingSensorData = new OptimalExposure::ImagingSensorData(cameraName, OptimalExposure::SENSORTYPE_MONOCHROME, OptimalExposure::GAIN_SELECTION_TYPE_NORMAL, *gainSelection, *aSensorGainReadNoiseVector);
    OptimalExposure::FileUtilitySensorData::writeSensorDataFile(anImagingSensorData);



    cameraName = "QHY268M-Mode0";  // The same read-noise is used in both Mono and Color?
    aSensorGainReadNoiseVector = new QVector<OptimalExposure::SensorGainReadNoise>();
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(0,7.25)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(5,7.1)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(10,6.78)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(15,6.74)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(20,6.74)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(25,2.8)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(30,2.63)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(35,2.7)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(40,2.57)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(45,2.55)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(50,2.52)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(55,2.35)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(60,2.07)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(65,2.05)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(70,2.04)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(75,2.06)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(80,2.03)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(85,2.01)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(90,2)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(95,1.99)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(100,2)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(105,1.97)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(110,1.96)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(115,1.95)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(120,1.94)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(125,1.93)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(130,1.92)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(135,1.89)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(140,1.9)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(145,1.89)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(150,1.85)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(155,1.82)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(160,1.81)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(165,1.8)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(170,1.79)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(175,1.75)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(180,1.74)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(185,1.7)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(190,1.68)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(195,1.67)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(200,1.62)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(205,1.59)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(210,1.56)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(215,1.53)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(220,1.48)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(225,1.44)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(230,1.41)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(235,1.36)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(240,1.32)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(245,1.26)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(250,1.17)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(255,1.12)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(260,1.03)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(265,0.92)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(270,0.79)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(275,0.62)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(280,0.6)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(285,0.7)));

    gainSelection = new QVector<int>( { 0, 285 } );
    anImagingSensorData = new OptimalExposure::ImagingSensorData(cameraName, OptimalExposure::SENSORTYPE_MONOCHROME, OptimalExposure::GAIN_SELECTION_TYPE_NORMAL, *gainSelection, *aSensorGainReadNoiseVector);
    OptimalExposure::FileUtilitySensorData::writeSensorDataFile(anImagingSensorData);


    cameraName = "QHY268M-Mode1";  // The same read-noise is used in both Mono and Color?
    aSensorGainReadNoiseVector = new QVector<OptimalExposure::SensorGainReadNoise>();

    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(5,3.6)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(10,3.58)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(15,3.64)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(20,3.63)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(25,3.63)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(30,3.58)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(35,3.49)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(40,3.39)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(45,3.43)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(50,3.53)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(55,3.43)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(60,1.69)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(65,1.69)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(70,1.67)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(75,1.67)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(80,1.63)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(85,1.6)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(90,1.57)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(95,1.53)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(100,1.25)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(105,1.22)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(110,1.22)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(115,1.21)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(120,1.2)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(125,1.2)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(130,1.19)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(135,1.18)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(140,1.17)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(145,1.15)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(150,1.14)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(155,1.14)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(160,1.13)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(165,1.12)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(170,1.11)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(175,1.09)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(180,1.09)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(185,1.08)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(190,1.06)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(195,1.06)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(200,1.03)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(205,1.01)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(210,0.98)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(215,0.99)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(220,0.96)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(225,0.97)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(230,0.92)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(235,0.93)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(240,0.87)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(245,0.87)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(250,0.8)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(255,0.79)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(260,0.8)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(265,0.7)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(270,0.71)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(275,0.7)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(280,0.69)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(285,0.69)));

    gainSelection = new QVector<int>( { 0, 285 } );
    anImagingSensorData = new OptimalExposure::ImagingSensorData(cameraName, OptimalExposure::SENSORTYPE_MONOCHROME, OptimalExposure::GAIN_SELECTION_TYPE_NORMAL, *gainSelection, *aSensorGainReadNoiseVector);
    OptimalExposure::FileUtilitySensorData::writeSensorDataFile(anImagingSensorData);

    cameraName = "QHY268M-Mode2";  // The same read-noise is used in both Mono and Color?
    aSensorGainReadNoiseVector = new QVector<OptimalExposure::SensorGainReadNoise>();
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(0,7.56)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(5,7.48)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(10,7.41)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(15,7.34)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(20,7.22)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(25,7.15)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(30,7.01)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(35,6.83)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(40,6.69)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(45,6.6)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(50,6.64)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(55,6.72)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(60,6.86)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(65,6.93)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(70,6.73)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(75,6.44)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(80,6.55)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(85,6.06)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(90,6.16)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(95,5.75)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(100,5.36)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(105,5.34)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(110,5.34)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(115,5.35)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(120,5.34)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(125,5.3)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(130,5.22)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(135,5.18)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(140,5.09)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(145,5.04)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(150,4.96)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(155,4.9)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(160,4.85)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(165,4.77)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(170,4.71)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(175,4.62)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(180,4.57)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(185,4.46)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(190,4.4)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(195,4.32)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(200,4.2)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(205,4.1)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(210,4.02)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(215,3.95)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(220,3.79)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(225,3.71)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(230,3.58)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(235,3.53)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(240,3.41)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(245,3.26)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(250,3.14)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(255,3.04)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(260,2.87)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(265,2.8)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(270,2.62)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(275,2.5)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(280,2.34)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(285,2.18)));

    gainSelection = new QVector<int>( { 0, 285 } );
    anImagingSensorData = new OptimalExposure::ImagingSensorData(cameraName, OptimalExposure::SENSORTYPE_MONOCHROME, OptimalExposure::GAIN_SELECTION_TYPE_NORMAL, *gainSelection, *aSensorGainReadNoiseVector);
    OptimalExposure::FileUtilitySensorData::writeSensorDataFile(anImagingSensorData);


    cameraName = "QHY268M-Mode3";  // The same read-noise is used in both Mono and Color?
    aSensorGainReadNoiseVector = new QVector<OptimalExposure::SensorGainReadNoise>();
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(0,5.89)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(5,5.82)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(10,5.82)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(15,5.73)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(20,5.66)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(25,5.58)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(30,5.44)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(35,5.39)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(40,5.26)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(45,5.17)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(50,5.17)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(55,5.25)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(60,5.34)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(65,5.43)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(70,5.33)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(75,5.08)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(80,5.05)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(85,4.81)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(90,4.85)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(95,4.55)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(100,4.26)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(105,4.25)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(110,4.23)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(115,4.26)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(120,4.26)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(125,4.23)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(130,4.18)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(135,4.15)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(140,4.08)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(145,4.09)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(150,4.05)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(155,4.01)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(160,3.98)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(165,3.96)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(170,3.91)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(175,3.87)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(180,3.83)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(185,3.8)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(190,3.76)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(195,3.68)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(200,3.64)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(205,3.57)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(210,3.51)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(215,3.44)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(220,3.39)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(225,3.33)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(230,3.25)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(235,3.2)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(240,3.13)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(245,3.03)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(250,2.99)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(255,2.87)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(260,2.77)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(265,2.72)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(270,2.58)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(275,2.5)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(280,2.37)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(285,2.22)));

    gainSelection = new QVector<int>( { 0, 285 } );
    anImagingSensorData = new OptimalExposure::ImagingSensorData(cameraName, OptimalExposure::SENSORTYPE_MONOCHROME, OptimalExposure::GAIN_SELECTION_TYPE_NORMAL, *gainSelection, *aSensorGainReadNoiseVector);
    OptimalExposure::FileUtilitySensorData::writeSensorDataFile(anImagingSensorData);


    cameraName = "ZWOASI-2600MM";
    aSensorGainReadNoiseVector = new QVector<OptimalExposure::SensorGainReadNoise>();
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(0,3.28)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(50,3.06)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(99,2.88)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(100,1.46)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(150,1.42)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(200,1.42)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(250,1.35)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(300,1.35)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(350,1.25)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(400,1.17)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(450,1.11)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(460,1.03)));

    gainSelection = new QVector<int>( { 0, 460 } );
    anImagingSensorData = new OptimalExposure::ImagingSensorData(cameraName, OptimalExposure::SENSORTYPE_MONOCHROME, OptimalExposure::GAIN_SELECTION_TYPE_NORMAL, *gainSelection, *aSensorGainReadNoiseVector);
    OptimalExposure::FileUtilitySensorData::writeSensorDataFile(anImagingSensorData);


    // DSLR Read-noise for ISO values data found at:   https://www.photonstophotos.net/Charts/RN_e.htm

    cameraName = "Nikon-D5000";

    aSensorGainReadNoiseVector = new QVector<OptimalExposure::SensorGainReadNoise>();
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(100, 7.727)));  // log2 value 2.95
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(126, 7.674)));  // log2 value 2.94
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(159, 7.727)));  // log2 value 2.95
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(200, 6.320)));  // log2 value 2.36)
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(251, 5.134)));  // log2 value 2.36)
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(318, 5.169)));  // log2 value 2.37
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(400, 4.532)));  // log2 value 2.18
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(503, 4.79)));  // log2 value 2.26
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(636, 5.169)));  // log2 value 2.37
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(800, 4.925)));  // log2 value 2.3
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(1006, 4.891)));  // log2 value 2.29
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(1273, 4.724)));  // log2 value 2.24
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(1600, 4.5)));  // log2 value 2.17
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(2011, 4.028)));  // log2 value 2.01
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(2546, 3.34)));  // log2 value 1.74
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(3200, 2.969)));  // log2 value 1.57
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(4022, 2.828)));  // log2 value 1.5
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(5091, 2.789)));  // log2 value 1.48
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(6400, 2.732)));  // log2 value 1.45



//  Unfortunately this data from www.photonstophotos.net does not align precisely with the discrete values for the ISO in the camera.
//  So, the proof of concept code, which needs a list of iso values will use an array from the file.
//  The following array would likely come from Ekos with getActiveChip()->getISOList() ?

    gainSelection = new QVector<int>( { 100, 125, 160, 250, 320, 400, 500, 640, 800, 1000, 1250, 1600, 2000, 2500, 3200, 4000, 5000, 6400 } );
    anImagingSensorData = new OptimalExposure::ImagingSensorData(cameraName, OptimalExposure::SENSORTYPE_COLOR, OptimalExposure::GAIN_SELECTION_TYPE_ISO_DISCRETE, *gainSelection, *aSensorGainReadNoiseVector);
    OptimalExposure::FileUtilitySensorData::writeSensorDataFile(anImagingSensorData);



    cameraName = "Canon EOS Ra";

    aSensorGainReadNoiseVector = new QVector<OptimalExposure::SensorGainReadNoise>();
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(100,9.063)));  // log2 value 3.18
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(126,10.2)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(159,5.58)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(200,4.96)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(251,5.46)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(318,3.14)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(400,3.18)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(503,3.34)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(636,2.41)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(800,2.41)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(1006,2.48)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(1273,1.92)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(1600,1.91)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(2011,1.91)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(2546,1.65)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(3200,1.67)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(4022,1.68)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(5091,1.4)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(6400,1.38)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(8045,1.36)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(10183,1.38)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(12800,1.38)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(16090,1.38)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(20366,1.34)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(25600,1.33)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(32180,1.35)));
    aSensorGainReadNoiseVector->push_back(*(new OptimalExposure::SensorGainReadNoise(40731,1.34)));



    gainSelection = new QVector<int>( { 100, 125, 160, 200, 250, 320, 400, 500, 640, 800, 1000, 1250, 1600, 2000, 2500, 3200, 4000, 5000, 6400, 8000, 10200, 12800, 16000, 20400, 25600, 32000, 40000  } );

    anImagingSensorData = new OptimalExposure::ImagingSensorData(cameraName, OptimalExposure::SENSORTYPE_COLOR, OptimalExposure::GAIN_SELECTION_TYPE_ISO_DISCRETE, *gainSelection, *aSensorGainReadNoiseVector);
    OptimalExposure::FileUtilitySensorData::writeSensorDataFile(anImagingSensorData);

*/

}

