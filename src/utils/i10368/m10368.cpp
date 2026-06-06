#include "i10368/m10368.h"
QVector<double> m10368::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
