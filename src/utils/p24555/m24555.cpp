#include "p24555/m24555.h"
QVector<double> m24555::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
