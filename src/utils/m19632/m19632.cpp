#include "m19632/m19632.h"
QVector<double> m19632::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
