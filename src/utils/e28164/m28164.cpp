#include "e28164/m28164.h"
QVector<double> m28164::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
