#include "p25595/m25595.h"
QVector<double> m25595::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
