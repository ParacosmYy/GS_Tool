#include "d35623/m35623.h"
QVector<double> m35623::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
