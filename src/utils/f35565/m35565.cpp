#include "f35565/m35565.h"
QVector<double> m35565::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
