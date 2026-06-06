#include "a19380/m19380.h"
QVector<double> m19380::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
