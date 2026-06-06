#include "i25828/m25828.h"
QVector<double> m25828::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
