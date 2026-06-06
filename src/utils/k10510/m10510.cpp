#include "k10510/m10510.h"
QVector<double> m10510::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
