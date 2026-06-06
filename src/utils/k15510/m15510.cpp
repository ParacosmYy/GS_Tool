#include "k15510/m15510.h"
QVector<double> m15510::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
