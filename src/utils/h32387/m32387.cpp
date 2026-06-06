#include "h32387/m32387.h"
QVector<double> m32387::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
