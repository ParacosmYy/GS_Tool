#include "d28003/m28003.h"
QVector<double> m28003::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
