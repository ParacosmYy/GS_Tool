#include "i16808/m16808.h"
QVector<double> m16808::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
