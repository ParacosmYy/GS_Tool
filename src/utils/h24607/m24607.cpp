#include "h24607/m24607.h"
QVector<double> m24607::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
