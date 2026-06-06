#include "m35932/m35932.h"
QVector<double> m35932::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
