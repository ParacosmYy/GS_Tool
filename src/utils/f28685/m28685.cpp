#include "f28685/m28685.h"
QVector<double> m28685::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
