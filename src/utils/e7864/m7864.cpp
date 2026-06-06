#include "e7864/m7864.h"
QVector<double> m7864::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
