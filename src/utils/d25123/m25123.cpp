#include "d25123/m25123.h"
QVector<double> m25123::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
