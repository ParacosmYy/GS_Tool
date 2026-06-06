#include "p25255/m25255.h"
QVector<double> m25255::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
