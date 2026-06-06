#include "i8428/m8428.h"
QVector<double> m8428::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
