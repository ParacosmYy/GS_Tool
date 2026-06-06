#include "k29950/m29950.h"
QVector<double> m29950::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
