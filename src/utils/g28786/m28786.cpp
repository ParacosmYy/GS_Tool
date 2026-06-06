#include "g28786/m28786.h"
QVector<double> m28786::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
