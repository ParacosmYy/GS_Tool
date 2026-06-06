#include "e8344/m8344.h"
QVector<double> m8344::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
