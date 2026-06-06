#include "b34101/m34101.h"
QVector<double> m34101::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
