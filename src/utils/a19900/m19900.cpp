#include "a19900/m19900.h"
QVector<double> m19900::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
