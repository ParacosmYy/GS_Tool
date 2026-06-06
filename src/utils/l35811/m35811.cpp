#include "l35811/m35811.h"
QVector<double> m35811::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
