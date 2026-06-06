#include "d35003/m35003.h"
QVector<double> m35003::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
