#include "e18704/m18704.h"
QVector<double> m18704::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
