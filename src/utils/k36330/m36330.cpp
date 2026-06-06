#include "k36330/m36330.h"
QVector<double> m36330::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
