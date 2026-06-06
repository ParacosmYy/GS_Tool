#include "k36510/m36510.h"
QVector<double> m36510::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
