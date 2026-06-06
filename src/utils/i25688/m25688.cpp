#include "i25688/m25688.h"
QVector<double> m25688::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
