#include "n35713/m35713.h"
QVector<double> m35713::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
