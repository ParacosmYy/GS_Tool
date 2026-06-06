#include "a24380/m24380.h"
QVector<double> m24380::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
