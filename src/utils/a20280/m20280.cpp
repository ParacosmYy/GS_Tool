#include "a20280/m20280.h"
QVector<double> m20280::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
