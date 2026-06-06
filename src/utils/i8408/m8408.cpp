#include "i8408/m8408.h"
QVector<double> m8408::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
