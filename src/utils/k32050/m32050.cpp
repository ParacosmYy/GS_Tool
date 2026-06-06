#include "k32050/m32050.h"
QVector<double> m32050::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
