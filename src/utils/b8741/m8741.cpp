#include "b8741/m8741.h"
QVector<double> m8741::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
