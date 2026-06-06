#include "o32094/m32094.h"
QVector<double> m32094::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
