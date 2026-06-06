#include "a16540/m16540.h"
QVector<double> m16540::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
