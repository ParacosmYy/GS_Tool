#include "n35053/m35053.h"
QVector<double> m35053::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
