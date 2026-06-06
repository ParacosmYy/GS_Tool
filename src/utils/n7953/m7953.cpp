#include "n7953/m7953.h"
QVector<double> m7953::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
