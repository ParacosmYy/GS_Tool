#include "p18275/m18275.h"
QVector<double> m18275::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
