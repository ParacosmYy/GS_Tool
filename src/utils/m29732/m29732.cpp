#include "m29732/m29732.h"
QVector<double> m29732::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
