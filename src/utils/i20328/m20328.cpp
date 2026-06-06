#include "i20328/m20328.h"
QVector<double> m20328::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
