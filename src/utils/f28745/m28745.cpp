#include "f28745/m28745.h"
QVector<double> m28745::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
