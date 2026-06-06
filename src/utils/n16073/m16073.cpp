#include "n16073/m16073.h"
QVector<double> m16073::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
