#include "m19832/m19832.h"
QVector<double> m19832::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
