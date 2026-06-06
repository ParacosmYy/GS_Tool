#include "i28208/m28208.h"
QVector<double> m28208::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
