#include "p25355/m25355.h"
QVector<double> m25355::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
