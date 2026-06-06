#include "n8313/m8313.h"
QVector<double> m8313::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
