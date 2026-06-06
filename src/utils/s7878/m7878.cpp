#include "s7878/m7878.h"
QVector<double> m7878::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
