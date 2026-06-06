#include "i16088/m16088.h"
QVector<double> m16088::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
