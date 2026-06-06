#include "k36850/m36850.h"
QVector<double> m36850::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
