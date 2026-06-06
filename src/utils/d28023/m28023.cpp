#include "d28023/m28023.h"
QVector<double> m28023::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
