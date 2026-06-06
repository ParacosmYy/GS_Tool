#include "c8582/m8582.h"
QVector<double> m8582::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
