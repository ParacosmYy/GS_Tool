#include "f26565/m26565.h"
QVector<double> m26565::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
