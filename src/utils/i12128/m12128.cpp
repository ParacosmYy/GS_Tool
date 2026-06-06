#include "i12128/m12128.h"
QVector<double> m12128::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
