#include "k25050/m25050.h"
QVector<double> m25050::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
