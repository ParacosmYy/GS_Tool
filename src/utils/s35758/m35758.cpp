#include "s35758/m35758.h"
QVector<double> m35758::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
