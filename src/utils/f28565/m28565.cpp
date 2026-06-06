#include "f28565/m28565.h"
QVector<double> m28565::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
