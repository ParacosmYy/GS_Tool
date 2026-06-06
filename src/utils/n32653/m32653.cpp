#include "n32653/m32653.h"
QVector<double> m32653::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
