#include "e35704/m35704.h"
QVector<double> m35704::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
