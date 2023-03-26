#include <QDebug>
#include <QApplication>
#include "subexposurecalculatorpoc.h"
#include "fileutilitysensordata.h"
#include "imagingsensordata.h"
#include "sensorgainreadnoise.h"

/*
 *
 *
 * https://www.astrobin.com/forum/c/astrophotography/deep-sky/robin-glover-talk-questioning-length-of-single-exposure/
 * http://astro.physics.uiowa.edu/~kaaret/2013f_29c137/Lab03_noise.html#:~:text=The%20read%20noise%20of%20the,removing%20hot%20and%20dead%20pixels
 *
 * Resource for DSLR read-noise:
 * https://www.photonstophotos.net/Charts/RN_ADU.htm
 *
 *
 * Coding References:
 *
 * https://doc.qt.io/qt-6/qtcharts-linechart-example.html
 * https://stackoverflow.com/questions/48362864/how-to-insert-qchartview-in-form-with-qt-designer
 * https://stackoverflow.com/questions/39128153/how-do-you-display-a-qchartview-in-a-custom-widget
 *
 *
 *
 */

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);

    // If we have a new camera to add from the file utility, uncomment this call, start and stop the app, and then comment out the call
    // OptimalExposure::FileUtilitySensorData::buildSensorDataFile();

    SubExposureCalculatorPOC w;

    w.show();
    return a.exec();
}





