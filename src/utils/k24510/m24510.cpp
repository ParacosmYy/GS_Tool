#include "k24510/m24510.h"
QVector<double> m24510::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
