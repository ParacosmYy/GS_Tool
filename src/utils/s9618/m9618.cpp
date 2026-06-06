#include "s9618/m9618.h"
QVector<double> m9618::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
