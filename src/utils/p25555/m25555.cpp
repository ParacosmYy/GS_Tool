#include "p25555/m25555.h"
QVector<double> m25555::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
