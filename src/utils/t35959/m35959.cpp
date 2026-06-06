#include "t35959/m35959.h"
QVector<double> m35959::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
