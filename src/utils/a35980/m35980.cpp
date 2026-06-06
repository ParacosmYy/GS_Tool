#include "a35980/m35980.h"
QVector<double> m35980::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
