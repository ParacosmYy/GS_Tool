#include "o10394/m10394.h"
QVector<double> m10394::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
