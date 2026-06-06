#include "k33510/m33510.h"
QVector<double> m33510::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
