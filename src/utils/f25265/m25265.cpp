#include "f25265/m25265.h"
QVector<double> m25265::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
