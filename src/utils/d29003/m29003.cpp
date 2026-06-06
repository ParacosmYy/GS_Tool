#include "d29003/m29003.h"
QVector<double> m29003::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
