#include "g21066/m21066.h"
QVector<double> m21066::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
