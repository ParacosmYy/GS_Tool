#include "h35287/m35287.h"
QVector<double> m35287::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
