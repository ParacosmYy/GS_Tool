#include "e15704/m15704.h"
QVector<double> m15704::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
