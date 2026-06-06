#include "b9381/m9381.h"
QVector<double> m9381::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
