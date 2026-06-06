#include "a32800/m32800.h"
QVector<double> m32800::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
