#include "e8124/m8124.h"
QVector<double> m8124::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
