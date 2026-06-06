#include "k20510/m20510.h"
QVector<double> m20510::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
