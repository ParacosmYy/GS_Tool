#include "n35753/m35753.h"
QVector<double> m35753::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
