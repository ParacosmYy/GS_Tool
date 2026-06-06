#include "b35021/m35021.h"
QVector<double> m35021::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
