#include "k25730/m25730.h"
QVector<double> m25730::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
