#include "a15580/m15580.h"
QVector<double> m15580::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
