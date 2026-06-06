#include "a25240/m25240.h"
QVector<double> m25240::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
