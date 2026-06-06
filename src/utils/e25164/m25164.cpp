#include "e25164/m25164.h"
QVector<double> m25164::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
