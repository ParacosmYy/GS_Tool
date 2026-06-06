#include "k28130/m28130.h"
QVector<double> m28130::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
