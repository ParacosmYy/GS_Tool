#include "a8680/m8680.h"
QVector<double> m8680::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
