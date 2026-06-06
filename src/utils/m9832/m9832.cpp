#include "m9832/m9832.h"
QVector<double> m9832::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
