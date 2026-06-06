#include "o8314/m8314.h"
QVector<double> m8314::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
