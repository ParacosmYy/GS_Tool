#include "n16833/m16833.h"
QVector<double> m16833::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
