#include "o9214/m9214.h"
QVector<double> m9214::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
