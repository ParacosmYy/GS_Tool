#include "b18261/m18261.h"
QVector<double> m18261::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
