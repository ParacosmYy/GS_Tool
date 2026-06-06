#include "f18385/m18385.h"
QVector<double> m18385::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
