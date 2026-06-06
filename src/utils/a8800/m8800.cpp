#include "a8800/m8800.h"
QVector<double> m8800::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
