#include "a15400/m15400.h"
QVector<double> m15400::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
