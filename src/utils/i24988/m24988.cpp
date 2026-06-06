#include "i24988/m24988.h"
QVector<double> m24988::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
