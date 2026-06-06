#include "i25128/m25128.h"
QVector<double> m25128::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
