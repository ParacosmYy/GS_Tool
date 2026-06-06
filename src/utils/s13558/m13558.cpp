#include "s13558/m13558.h"
QVector<double> m13558::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
