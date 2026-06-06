#include "m29232/m29232.h"
QVector<double> m29232::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
