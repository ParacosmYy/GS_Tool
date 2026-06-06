#include "f28585/m28585.h"
QVector<double> m28585::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
