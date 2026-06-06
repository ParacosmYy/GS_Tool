#include "q16896/m16896.h"
QVector<double> m16896::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
