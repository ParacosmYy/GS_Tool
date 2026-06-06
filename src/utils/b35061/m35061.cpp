#include "b35061/m35061.h"
QVector<double> m35061::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
