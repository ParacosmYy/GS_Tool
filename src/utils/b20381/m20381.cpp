#include "b20381/m20381.h"
QVector<double> m20381::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
