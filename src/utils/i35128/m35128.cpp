#include "i35128/m35128.h"
QVector<double> m35128::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
