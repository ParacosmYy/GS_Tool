#include "d35803/m35803.h"
QVector<double> m35803::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
