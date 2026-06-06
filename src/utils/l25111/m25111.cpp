#include "l25111/m25111.h"
QVector<double> m25111::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
