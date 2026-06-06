#include "o18014/m18014.h"
QVector<double> m18014::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
