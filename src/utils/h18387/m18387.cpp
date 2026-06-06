#include "h18387/m18387.h"
QVector<double> m18387::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
