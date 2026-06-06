#include "k36050/m36050.h"
QVector<double> m36050::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
