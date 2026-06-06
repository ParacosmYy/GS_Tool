#include "n8533/m8533.h"
QVector<double> m8533::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
