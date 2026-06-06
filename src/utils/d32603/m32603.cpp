#include "d32603/m32603.h"
QVector<double> m32603::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
