#include "a32020/m32020.h"
QVector<double> m32020::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
