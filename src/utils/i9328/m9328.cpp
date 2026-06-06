#include "i9328/m9328.h"
QVector<double> m9328::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
