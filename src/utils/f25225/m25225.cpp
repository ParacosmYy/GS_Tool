#include "f25225/m25225.h"
QVector<double> m25225::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
