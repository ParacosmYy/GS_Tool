#include "k23010/m23010.h"
QVector<double> m23010::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
