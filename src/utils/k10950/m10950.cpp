#include "k10950/m10950.h"
QVector<double> m10950::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
