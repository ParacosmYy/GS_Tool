#include "n8813/m8813.h"
QVector<double> m8813::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
