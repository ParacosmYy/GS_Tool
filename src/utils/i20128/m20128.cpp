#include "i20128/m20128.h"
QVector<double> m20128::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
