#include "d29103/m29103.h"
QVector<double> m29103::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
