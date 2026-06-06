#include "m19092/m19092.h"
QVector<double> m19092::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
