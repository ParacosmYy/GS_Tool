#include "b25621/m25621.h"
QVector<double> m25621::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
