#include "a23100/m23100.h"
QVector<double> m23100::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
