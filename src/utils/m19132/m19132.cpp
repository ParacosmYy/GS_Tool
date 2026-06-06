#include "m19132/m19132.h"
QVector<double> m19132::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
