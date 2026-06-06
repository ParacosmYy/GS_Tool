#include "l7851/m7851.h"
QVector<double> m7851::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
