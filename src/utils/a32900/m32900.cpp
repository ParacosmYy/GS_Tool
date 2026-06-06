#include "a32900/m32900.h"
QVector<double> m32900::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
