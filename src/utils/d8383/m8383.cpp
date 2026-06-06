#include "d8383/m8383.h"
QVector<double> m8383::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
