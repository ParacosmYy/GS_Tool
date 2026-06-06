#include "k21510/m21510.h"
QVector<double> m21510::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
