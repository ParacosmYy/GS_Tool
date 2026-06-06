#include "i8808/m8808.h"
QVector<double> m8808::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
