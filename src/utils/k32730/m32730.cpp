#include "k32730/m32730.h"
QVector<double> m32730::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
