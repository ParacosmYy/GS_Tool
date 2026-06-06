#include "a35360/m35360.h"
QVector<double> m35360::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
