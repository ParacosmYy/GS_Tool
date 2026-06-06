#include "f18705/m18705.h"
QVector<double> m18705::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
