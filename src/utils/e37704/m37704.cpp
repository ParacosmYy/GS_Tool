#include "e37704/m37704.h"
QVector<double> m37704::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
