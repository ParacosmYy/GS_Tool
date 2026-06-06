#include "e8424/m8424.h"
QVector<double> m8424::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
