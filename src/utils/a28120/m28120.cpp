#include "a28120/m28120.h"
QVector<double> m28120::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
