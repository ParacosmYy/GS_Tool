#include "m30312/m30312.h"
QVector<double> m30312::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
