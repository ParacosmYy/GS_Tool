#include "o16154/m16154.h"
QVector<double> m16154::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
