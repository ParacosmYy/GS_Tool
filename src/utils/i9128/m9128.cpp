#include "i9128/m9128.h"
QVector<double> m9128::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
