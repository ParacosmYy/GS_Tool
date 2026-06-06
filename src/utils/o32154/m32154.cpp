#include "o32154/m32154.h"
QVector<double> m32154::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
