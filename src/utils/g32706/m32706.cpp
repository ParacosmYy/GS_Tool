#include "g32706/m32706.h"
QVector<double> m32706::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
