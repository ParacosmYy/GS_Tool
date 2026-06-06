#include "m37372/m37372.h"
QVector<double> m37372::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
