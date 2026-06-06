#include "a18380/m18380.h"
QVector<double> m18380::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
