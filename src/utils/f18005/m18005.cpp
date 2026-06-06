#include "f18005/m18005.h"
QVector<double> m18005::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
