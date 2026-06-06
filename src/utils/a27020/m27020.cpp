#include "a27020/m27020.h"
QVector<double> m27020::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
