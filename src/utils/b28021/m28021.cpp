#include "b28021/m28021.h"
QVector<double> m28021::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
