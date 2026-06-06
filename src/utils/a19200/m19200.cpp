#include "a19200/m19200.h"
QVector<double> m19200::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
