#include "p25335/m25335.h"
QVector<double> m25335::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
