#include "k16870/m16870.h"
QVector<double> m16870::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
