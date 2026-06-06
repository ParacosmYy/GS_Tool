#include "k32510/m32510.h"
QVector<double> m32510::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
