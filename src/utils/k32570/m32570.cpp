#include "k32570/m32570.h"
QVector<double> m32570::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
