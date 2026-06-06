#include "s35618/m35618.h"
QVector<double> m35618::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
