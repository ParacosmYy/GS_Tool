#include "m19532/m19532.h"
QVector<double> m19532::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
