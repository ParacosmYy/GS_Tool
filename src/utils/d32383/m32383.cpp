#include "d32383/m32383.h"
QVector<double> m32383::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
