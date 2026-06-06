#include "b15381/m15381.h"
QVector<double> m15381::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
