#include "i16048/m16048.h"
QVector<double> m16048::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
