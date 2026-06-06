#include "m32412/m32412.h"
QVector<double> m32412::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
