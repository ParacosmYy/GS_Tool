#include "k36010/m36010.h"
QVector<double> m36010::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
