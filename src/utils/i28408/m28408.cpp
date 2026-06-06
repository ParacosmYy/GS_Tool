#include "i28408/m28408.h"
QVector<double> m28408::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
