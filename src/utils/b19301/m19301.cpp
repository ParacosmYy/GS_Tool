#include "b19301/m19301.h"
QVector<double> m19301::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
