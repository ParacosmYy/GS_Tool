#include "r35017/m35017.h"
QVector<double> m35017::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
