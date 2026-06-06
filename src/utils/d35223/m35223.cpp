#include "d35223/m35223.h"
QVector<double> m35223::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
