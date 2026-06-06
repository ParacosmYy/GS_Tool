#include "a21020/m21020.h"
QVector<double> m21020::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
