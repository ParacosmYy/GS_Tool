#include "a20600/m20600.h"
QVector<double> m20600::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
