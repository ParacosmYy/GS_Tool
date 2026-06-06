#include "k36350/m36350.h"
QVector<double> m36350::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
