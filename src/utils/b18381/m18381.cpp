#include "b18381/m18381.h"
QVector<double> m18381::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
