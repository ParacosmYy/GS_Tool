#include "b9741/m9741.h"
QVector<double> m9741::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
