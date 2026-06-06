#include "f18925/m18925.h"
QVector<double> m18925::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
