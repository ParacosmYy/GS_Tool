#include "c16882/m16882.h"
QVector<double> m16882::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
