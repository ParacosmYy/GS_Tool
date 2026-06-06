#include "d35203/m35203.h"
QVector<double> m35203::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
