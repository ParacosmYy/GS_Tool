#include "f28005/m28005.h"
QVector<double> m28005::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
