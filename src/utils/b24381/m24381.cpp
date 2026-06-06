#include "b24381/m24381.h"
QVector<double> m24381::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
