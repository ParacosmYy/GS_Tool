#include "m20272/m20272.h"
QVector<double> m20272::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
