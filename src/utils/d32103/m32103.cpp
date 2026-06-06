#include "d32103/m32103.h"
QVector<double> m32103::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
