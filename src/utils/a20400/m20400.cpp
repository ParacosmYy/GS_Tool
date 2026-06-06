#include "a20400/m20400.h"
QVector<double> m20400::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
