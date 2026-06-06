#include "d7863/m7863.h"
QVector<double> m7863::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
