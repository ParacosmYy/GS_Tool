#include "k25830/m25830.h"
QVector<double> m25830::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
