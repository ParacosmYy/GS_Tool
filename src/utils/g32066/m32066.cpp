#include "g32066/m32066.h"
QVector<double> m32066::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
