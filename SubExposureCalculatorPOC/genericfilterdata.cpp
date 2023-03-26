#include "genericfilterdata.h"
#include <QAbstractItemModel>

OptimalExposure::GenericFilterData::GenericFilterData()
{

}


namespace OptimalExposure {
const QString &GenericFilterData::getFilterId() const
{
    return filterId;
}

void GenericFilterData::setFilterId(const QString &newFilterId)
{
    filterId = newFilterId;
}

const QString &GenericFilterData::getFilterDescription() const
{
    return filterDescription;
}

void GenericFilterData::setFilterDescription(const QString &newFilterDescription)
{
    filterDescription = newFilterDescription;
}

int GenericFilterData::getFilterBandPassWidth() const
{
    return filterBandPassWidth;
}

void GenericFilterData::setFilterBandPassWidth(int newFilterBandPassWidth)
{
    filterBandPassWidth = newFilterBandPassWidth;
}

double GenericFilterData::getFilterCompensation() const
{
    return ((double)filterBandPassWidth / (double) 300);
}

}
