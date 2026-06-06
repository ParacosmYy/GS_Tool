#include "a35820/m35820.h"
QVector<double> m35820::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
