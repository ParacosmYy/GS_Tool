#include "f8565/m8565.h"
QVector<double> m8565::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
