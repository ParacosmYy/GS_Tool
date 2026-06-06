#include "m15832/m15832.h"
QVector<double> m15832::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
