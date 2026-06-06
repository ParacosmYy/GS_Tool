#include "i15208/m15208.h"
QVector<double> m15208::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
