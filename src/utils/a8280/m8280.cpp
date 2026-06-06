#include "a8280/m8280.h"
QVector<double> m8280::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
