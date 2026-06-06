#include "m20832/m20832.h"
QVector<double> m20832::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
