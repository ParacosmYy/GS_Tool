#include "k16510/m16510.h"
QVector<double> m16510::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
