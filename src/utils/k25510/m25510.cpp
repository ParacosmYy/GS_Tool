#include "k25510/m25510.h"
QVector<double> m25510::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
