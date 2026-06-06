#include "k16890/m16890.h"
QVector<double> m16890::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
