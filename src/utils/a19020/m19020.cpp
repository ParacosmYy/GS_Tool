#include "a19020/m19020.h"
QVector<double> m19020::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
