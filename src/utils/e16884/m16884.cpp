#include "e16884/m16884.h"
QVector<double> m16884::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
