#include "s15038/m15038.h"
QVector<double> m15038::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
