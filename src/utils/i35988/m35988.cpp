#include "i35988/m35988.h"
QVector<double> m35988::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
