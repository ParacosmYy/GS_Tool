#include "d7883/m7883.h"
QVector<double> m7883::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
