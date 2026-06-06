#include "k36750/m36750.h"
QVector<double> m36750::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
