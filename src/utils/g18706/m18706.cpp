#include "g18706/m18706.h"
QVector<double> m18706::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
