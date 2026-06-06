#include "n8513/m8513.h"
QVector<double> m8513::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
