#include "e24704/m24704.h"
QVector<double> m24704::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
