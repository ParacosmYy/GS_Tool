#include "k13050/m13050.h"
QVector<double> m13050::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
