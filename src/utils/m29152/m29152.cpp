#include "m29152/m29152.h"
QVector<double> m29152::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
